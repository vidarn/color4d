#include "c4d.h"
#include "toolsculpting.h"
#include "c4d_symbols.h"

//#define USE_TIMER

#define ID_SCULPTING_TOOL	450000250

class SculptingTool : public DescriptionToolData
{
public:
	SculptingTool();
	virtual ~SculptingTool();

private:
	virtual LONG GetToolPluginId() { return ID_SCULPTING_TOOL; }
	virtual const String GetResourceSymbol() { return String("ToolSculpting"); }
	virtual Bool InitTool(BaseDocument *pDoc, BaseContainer &data, BaseThread *bt);
	virtual void FreeTool(BaseDocument *pDoc, BaseContainer &data);
	virtual void InitDefaultSettings(BaseDocument *doc, BaseContainer &data);
	virtual Bool GetCursorInfo(BaseDocument *pDoc, BaseContainer &data, BaseDraw *pDraw, Real x, Real y, BaseContainer &bc);
	virtual Bool MouseInput(BaseDocument *pDoc, BaseContainer &data, BaseDraw *pDraw, EditorWindow *win, const BaseContainer &msg);

	Bool ValidateViewport(BaseDocument* pDoc, BaseDraw* pDraw);
	void UpdateObject(Vector* pPoints, Real rMouseX, Real rMouseY, Real rRadius, const Vector &vDelta, Bool bAllowVBOUpdate, ULONG ulUpdateFlags);

	ViewportSelect* m_pViewportSelect;
	Bool m_bViewportValid;
	BaseDraw* m_pLastBaseDraw;
	LONG m_lLastWidth, m_lLastHeight;
	PolygonObject* m_pLastObject;
	LONG m_lLastDirty;
	ULONG m_lLastEditorCameraDirty;
	BaseDocument* m_pDoc;
	Real m_rLastMouseX, m_rLastMouseY;
};

SculptingTool::SculptingTool()
{
	m_pViewportSelect = NULL;
	m_pLastBaseDraw = NULL;
	m_bViewportValid = FALSE;
	m_pLastObject = NULL;
	m_lLastDirty = 0;
	m_pDoc = NULL;
}

SculptingTool::~SculptingTool()
{
	ViewportSelect::Free(m_pViewportSelect);
}

Bool SculptingTool::InitTool(BaseDocument *pDoc, BaseContainer &data, BaseThread *bt)
{
	if (!DescriptionToolData::InitTool(pDoc,data,bt)) return FALSE;

	m_rLastMouseX = m_rLastMouseY = -1.0f;
	m_bViewportValid = FALSE;
	m_pLastBaseDraw = NULL;
	ViewportSelect::Free(m_pViewportSelect);
	m_lLastWidth = m_lLastHeight = -1;
	m_pLastObject = NULL;
	m_lLastDirty = 0;
	m_pDoc = NULL;
	return TRUE;
}

void SculptingTool::FreeTool(BaseDocument *pDoc, BaseContainer &data)
{
	m_pLastObject = NULL;
	m_lLastDirty = 0;
	ViewportSelect::Free(m_pViewportSelect);
	DescriptionToolData::FreeTool(pDoc,data);
}

void SculptingTool::InitDefaultSettings(BaseDocument *doc, BaseContainer &data)
{
	data.SetReal(SCULPTING_RADIUS, 40.0);
	data.SetVector(SCULPTING_VECTOR, Vector(0.0, 50.0, 0.0));
	data.SetBool(SCULPTING_DO_VBO_UPDATE, FALSE);
	DescriptionToolData::InitDefaultSettings(doc,data);
}

Bool SculptingTool::GetCursorInfo(BaseDocument *pDoc, BaseContainer &data, BaseDraw *pDraw, Real x, Real y, BaseContainer &bc)
{
	if (bc.GetId()!=BFM_CURSORINFO_REMOVE)
	{
		if (!pDoc)
			return TRUE;

		if (!pDoc->IsCacheBuilt()) return TRUE;

		if (pDraw != pDoc->GetActiveBaseDraw())
			return TRUE;

		if (!ValidateViewport(pDoc, pDraw))
			return FALSE;
	}
	else
	{
		SpecialEventAdd(EVMSG_UPDATEHIGHLIGHT);	// FIX[40724]
	}

	return TRUE;
}

Bool SculptingTool::MouseInput(BaseDocument *pDoc, BaseContainer &data, BaseDraw *pDraw, EditorWindow *win, const BaseContainer &msg)
{
	if (!pDoc)
		return TRUE;
	if (pDraw != pDoc->GetActiveBaseDraw())
		return TRUE;

	if(!m_pLastObject)
		return TRUE;

	LONG lMouseX = msg.GetLong(BFM_INPUT_X);
	LONG lMouseY = msg.GetLong(BFM_INPUT_Y);
	LONG lLeft, lTop, lRight, lBottom;
	pDraw->GetFrame(&lLeft, &lTop, &lRight, &lBottom);

	if (!ValidateViewport(pDoc, pDraw))
		return FALSE;

	Real rMouseX = (Real)lMouseX;	Real rMouseY = (Real)lMouseY;
	Real dx, dy;
	Bool bFirst = TRUE;
	BaseContainer bcDevice;
	Vector* pvPoints = m_pLastObject->GetPointW();
	Real rRadius = data.GetReal(SCULPTING_RADIUS);
	Bool bAllowVBOUpdate = data.GetBool(SCULPTING_DO_VBO_UPDATE);
	Vector vMove = data.GetVector(SCULPTING_VECTOR);
	ULONG ulUpdateFlags;

	if ((ulUpdateFlags = m_pLastObject->VBOInitUpdate(pDraw)) == 0)
	{
		// update the object so that triangle strips are deleted
		m_pLastObject->Message(MSG_UPDATE);
		DrawViews(DRAWFLAGS_ONLY_ACTIVE_VIEW|DRAWFLAGS_NO_THREAD|DRAWFLAGS_NO_ANIMATION);
		if ((ulUpdateFlags = m_pLastObject->VBOInitUpdate(pDraw)) == 0)
			return FALSE;
	}

	win->MouseDragStart(KEY_MLEFT, rMouseX, rMouseY, MOUSEDRAGFLAGS_DONTHIDEMOUSE);
	pDoc->StartUndo();
	pDoc->AddUndo(UNDOTYPE_CHANGE, m_pLastObject);
	while (win->MouseDrag(&dx, &dy, &bcDevice) == MOUSEDRAGRESULT_CONTINUE)
	{
		if (!bFirst && dx == 0.0f && dy == 0.0f)
			continue;
		bFirst = FALSE;
		rMouseX += dx;
		rMouseY += dy;
		UpdateObject(pvPoints, rMouseX, rMouseY, rRadius, vMove, bAllowVBOUpdate, ulUpdateFlags);
#ifdef USE_TIMER
		LONG lTimer = GeGetTimer();
#endif
		DrawViews(DRAWFLAGS_ONLY_ACTIVE_VIEW|DRAWFLAGS_NO_THREAD|DRAWFLAGS_NO_ANIMATION);
#ifdef USE_TIMER
		GePrint("DrawViews: " + LongToString(GeGetTimer() - lTimer));
#endif
	}
	win->MouseDragEnd();
	m_pLastObject->Message(MSG_UPDATE);
	pDoc->EndUndo();

	m_pLastObject->VBOFreeUpdate();

	DrawViews(DRAWFLAGS_ONLY_ACTIVE_VIEW|DRAWFLAGS_NO_THREAD|DRAWFLAGS_NO_ANIMATION);
	SpecialEventAdd(EVMSG_UPDATEHIGHLIGHT);
	return TRUE;
}

Bool SculptingTool::ValidateViewport(BaseDocument* pDoc, BaseDraw* pDraw)
{
	LONG lLeft, lTop, lRight, lBottom;
	pDraw->GetFrame(&lLeft, &lTop, &lRight, &lBottom);
	lRight = lRight - lLeft + 1;
	lBottom = lBottom - lTop + 1;
	BaseObject* pCam = pDraw->GetSceneCamera(pDoc) ? pDraw->GetSceneCamera(pDoc) : pDraw->GetEditorCamera();
	LONG lDirty;
	BaseObject* pSelected;

	if (!pDoc->IsCacheBuilt())
		return FALSE;

	pSelected = pDoc->GetActiveObject();
	if (pSelected && pSelected->IsInstanceOf(Opolygon))
	{
		lDirty = pSelected->GetDirty(DIRTYFLAGS_DATA | DIRTYFLAGS_MATRIX);
	}
	else
	{
		pSelected = NULL;
		lDirty = 0;
		m_pLastObject = NULL;
	}

	if (pDraw != m_pLastBaseDraw)
		m_bViewportValid = FALSE;
	if (m_bViewportValid && (lRight != m_lLastWidth || lBottom != m_lLastHeight))
		m_bViewportValid = FALSE;
	if (m_bViewportValid && !(pSelected == m_pLastObject && lDirty == m_lLastDirty))
		m_bViewportValid = FALSE;
	if (m_bViewportValid && m_lLastEditorCameraDirty != pCam->GetDirty(DIRTYFLAGS_MATRIX | DIRTYFLAGS_DATA))
		m_bViewportValid = FALSE;
	if (m_bViewportValid && m_pDoc != pDoc)
		m_bViewportValid = FALSE;

	if (!m_bViewportValid)
	{
		ViewportSelect::Free(m_pViewportSelect);
	}

	if (!m_pViewportSelect && pSelected)
	{
		m_pLastObject = (PolygonObject*)pSelected;
		m_lLastDirty = lDirty;
		m_lLastWidth = lRight;
		m_lLastHeight = lBottom;
		m_pLastBaseDraw = pDraw;
		m_pViewportSelect = ViewportSelect::Alloc();
		m_bViewportValid = TRUE;
		m_rLastMouseX = m_rLastMouseY = -1.0f;
		m_lLastEditorCameraDirty = pCam->GetDirty(DIRTYFLAGS_DATA | DIRTYFLAGS_MATRIX);
		m_pDoc = pDoc;
		if (!m_pViewportSelect)
			return FALSE;
		if (m_pLastObject)
		{
			if (!m_pViewportSelect->Init(m_lLastWidth, m_lLastHeight, pDraw, m_pLastObject, Mpoints, FALSE, VIEWPORTSELECTFLAGS_0))
				return FALSE;
		}
	}
	return TRUE;
}

Real SQR(Real r)
{
	return r * r;
}

void SculptingTool::UpdateObject(Vector* pvPoints, Real rMouseX, Real rMouseY, Real rRadius, const Vector &vDelta, Bool bAllowVBOUpdate, ULONG ulUpdateFlags)
{
	LONG x1 = LMax(0, (LONG)(rMouseX - rRadius));
	LONG x2 = LMin(m_lLastWidth - 1, (LONG)(rMouseX + rRadius));
	LONG y1 = LMax(0, (LONG)(rMouseY - rRadius));
	LONG y2 = LMin(m_lLastHeight - 1, (LONG)(rMouseY + rRadius));
	LONG x, y;
	const GlVertexBufferAttributeInfo* pVertexInfo;
	Real rRadSqr = rRadius * rRadius;

	if (rRadSqr < 1.0)
		return;

#ifdef USE_TIMER
	LONG lTimer = GeGetTimer(), lTimer1 = lTimer;
#endif
	if (bAllowVBOUpdate)
	{
		if (!m_pLastObject->VBOStartUpdate(m_pLastBaseDraw, VBWriteOnly, TRUE))
			bAllowVBOUpdate = FALSE;
		else
		{
#ifdef USE_TIMER
			GePrint("VBOStartUpdate: " + LongToString(GeGetTimer() - lTimer));
#endif
		}
	}

#ifdef USE_TIMER
	lTimer = GeGetTimer();
#endif
	if (ulUpdateFlags & POLYOBJECT_VBO_VERTEX)
		pVertexInfo = m_pLastObject->VBOUpdateVectorGetAttribute(POLYOBJECT_VBO_VERTEX);
	else
		pVertexInfo = NULL;
	for (x = x1; x <= x2; x++)
	{
		for (y = y1; y <= y2; y++)
		{
			Real rSqrDist = SQR(x - rMouseX) + SQR(y - rMouseY);
			if (rSqrDist > rRadSqr)
				continue;

			ViewportPixel* pPixel = m_pViewportSelect->GetPixelInfoPoint(x, y);
			while (pPixel)
			{
				if (pPixel->op == m_pLastObject)
				{
					pvPoints[pPixel->i] += vDelta * Smoothstep(0.0, 1.0, (1.0 - rSqrDist / rRadSqr));
					if (bAllowVBOUpdate)
					{
						if (pVertexInfo)
							m_pLastObject->VBOUpdateVector(pPixel->i, pvPoints[pPixel->i].ToSV(), pVertexInfo);
						// updating the normals is left as a task for the user ;-)
					}
				}
				pPixel = pPixel->next;
			}
		}
	}

#ifdef USE_TIMER
	if (bAllowVBOUpdate)
		GePrint("VBOUpdateVector: " + LongToString(GeGetTimer() - lTimer));
#endif

	if (bAllowVBOUpdate)
	{
		m_pLastObject->Message(MSG_UPDATE); // must be called before VBOEndUpdate
#ifdef USE_TIMER
		lTimer = GeGetTimer();
#endif

		m_pLastObject->VBOEndUpdate(m_pLastBaseDraw);

#ifdef USE_TIMER
		GePrint("VBOEndUpdate: " + LongToString(GeGetTimer() - lTimer));
#endif
	}
	else
		m_pLastObject->Message(MSG_UPDATE);

#ifdef USE_TIMER
	GePrint("SculptingTool::UpdateObject " + LongToString(GeGetTimer() - lTimer1));
#endif
}


Bool RegisterSculptingTool()
{
	return RegisterToolPlugin(ID_SCULPTING_TOOL, GeLoadString(IDS_SCULPTING_TOOL), PLUGINFLAG_TOOL_NO_WIREFRAME, NULL, GeLoadString(IDS_SCULPTING_TOOL), gNew SculptingTool);
}
