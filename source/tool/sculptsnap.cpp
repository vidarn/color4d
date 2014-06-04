/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2012 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

#include "c4d.h"
#include "toolsculpting.h"
#include "c4d_symbols.h"
#include "lib_sculpt.h"
#include "lib_modeling.h"
#include "lib_ngon.h"
#include "toolsculptsnap.h"

#define ID_SCULPSNAP_TOOL	1027981 

class SculptSnapTool : public DescriptionToolData
{
public:
	SculptSnapTool();
	virtual ~SculptSnapTool();

private:
	virtual LONG GetToolPluginId() { return ID_SCULPSNAP_TOOL; }
	virtual const String GetResourceSymbol() { return String("ToolSculptSnap"); }
	virtual Bool InitTool(BaseDocument *pDoc, BaseContainer &data, BaseThread *bt);
	virtual void FreeTool(BaseDocument *pDoc, BaseContainer &data);
	virtual void InitDefaultSettings(BaseDocument *doc, BaseContainer &data);
	virtual Bool GetCursorInfo(BaseDocument *pDoc, BaseContainer &data, BaseDraw *pDraw, Real x, Real y, BaseContainer &bc);
	virtual Bool MouseInput(BaseDocument *pDoc, BaseContainer &data, BaseDraw *pDraw, EditorWindow *win, const BaseContainer &msg);
	virtual TOOLDRAW  Draw(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, BaseDrawHelp *bh, BaseThread *bt,TOOLDRAWFLAGS flags);

	SculptObject* m_pLastObject;
	Real m_rLastMouseX, m_rLastMouseY;
};

SculptSnapTool::SculptSnapTool()
{
}

SculptSnapTool::~SculptSnapTool()
{
}

Bool SculptSnapTool::InitTool(BaseDocument *pDoc, BaseContainer &data, BaseThread *bt)
{
	if (!DescriptionToolData::InitTool(pDoc,data,bt)) return FALSE;

	m_rLastMouseX = m_rLastMouseY = -1.0f;
	m_pLastObject = NULL;
	return TRUE;
}

void SculptSnapTool::FreeTool(BaseDocument *pDoc, BaseContainer &data)
{
	m_pLastObject = NULL;
	DescriptionToolData::FreeTool(pDoc,data);
}

void SculptSnapTool::InitDefaultSettings(BaseDocument *doc, BaseContainer &data)
{
	data.SetReal(SCULPTSNAP_POLYGONSIZE, 5.0);
	DescriptionToolData::InitDefaultSettings(doc,data);
}

Bool SculptSnapTool::GetCursorInfo(BaseDocument *pDoc, BaseContainer &data, BaseDraw *pDraw, Real x, Real y, BaseContainer &bc)
{
	if (bc.GetId()!=BFM_CURSORINFO_REMOVE)
	{
		if (!pDoc)
			return TRUE;

		if (pDraw != pDoc->GetActiveBaseDraw())
			return TRUE;

		SculptObject *pSculpt = GetSelectedSculptObject(pDoc);
		if(!pSculpt) 
		{
			m_pLastObject = NULL;
			return TRUE;
		}

		if(pSculpt != m_pLastObject)
		{
			m_pLastObject = pSculpt;

			//Unfreeze the object so that we can use collision detection
			if(m_pLastObject->IsFrozen())
			{
				m_pLastObject->SetFrozen(FALSE);
			}

			//Request a collision update
			m_pLastObject->NeedCollisionUpdate();
		}

		if(m_pLastObject)
		{
			//Update the collision data
			m_pLastObject->UpdateCollision();
		}
	}
	else
	{
		SpecialEventAdd(EVMSG_UPDATEHIGHLIGHT);	
	}

	return TRUE;
}

TOOLDRAW SculptSnapTool::Draw(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, BaseDrawHelp *bh, BaseThread *bt,TOOLDRAWFLAGS flags)
{
	return TOOLDRAW_HIGHLIGHTS;
}


Bool SculptSnapTool::MouseInput(BaseDocument *pDoc, BaseContainer &data, BaseDraw *pDraw, EditorWindow *win, const BaseContainer &msg)
{
	if (!pDoc)
		return TRUE;
	if (pDraw != pDoc->GetActiveBaseDraw())
		return TRUE;

	if(!m_pLastObject) return TRUE;

	Real distance = data.GetReal(SCULPTSNAP_POLYGONSIZE);

	LONG lMouseX = msg.GetLong(BFM_INPUT_X);
	LONG lMouseY = msg.GetLong(BFM_INPUT_Y);
	LONG lLeft, lTop, lRight, lBottom;
	pDraw->GetFrame(&lLeft, &lTop, &lRight, &lBottom);

	Real rMouseX = (Real)lMouseX;
	Real rMouseY = (Real)lMouseY;
	Real dx, dy;
	Bool bFirst = TRUE;
	BaseContainer bcDevice;

	PolygonObject *poly = PolygonObject::Alloc(0,0);
	if (!poly) return FALSE;

	pDoc->StartUndo();

	pDoc->InsertObject(poly, NULL, NULL);
	pDoc->AddUndo(UNDOTYPE_NEW,poly);

	pDoc->EndUndo();

	poly->SetBit(BIT_ACTIVE);

	EventAdd();

	AutoAlloc<Modeling> mod;
	if (!mod || !mod->InitObject(poly)) return FALSE;

	Vector p1, p2, p3, p4;
	LONG index1, index2, index3, index4;
	index1=index2=index3=index4=0;

	Bool firstHit = FALSE;
	Bool firstPointDone = FALSE;
	Vector hitPoint;

	win->MouseDragStart(KEY_MLEFT, rMouseX, rMouseY, MOUSEDRAGFLAGS_DONTHIDEMOUSE);
	while (win->MouseDrag(&dx, &dy, &bcDevice) == MOUSEDRAGRESULT_CONTINUE)
	{
		if (!bFirst && dx == 0.0f && dy == 0.0f)
			continue;

		bFirst = FALSE;

		rMouseX += dx;
		rMouseY += dy;

		if(!firstHit)
		{
			SculptHitData hitData;
			if(m_pLastObject->Hit(pDraw,rMouseX,rMouseY,hitData))
			{
				firstHit = TRUE;
				hitPoint = hitData.localHitPoint + hitData.localHitNormal.ToLV();
				continue;
			}
		}

		if(firstHit)
		{
			SculptHitData hitData;
			if(m_pLastObject->Hit(pDraw,rMouseX,rMouseY,hitData))
			{
				Vector normal = hitData.localHitNormal.ToLV();
				Vector newP = hitData.localHitPoint + hitData.localHitNormal.ToLV();
				Vector diff = hitPoint - newP;
				Real len = diff.GetLength();
				if(len > distance)
				{
					Vector cross = Cross(normal,diff.GetNormalized());
					Vector gap = cross*distance*0.5;
					if(!firstPointDone)
					{
						firstPointDone = TRUE;
						p1 = hitPoint - gap;
						p2 = hitPoint + gap;

						index1 = mod->AddPoint(poly, p1);
						index2 = mod->AddPoint(poly, p2);
					}

					p3 = newP + gap;
					p4 = newP - gap;

					index3 = mod->AddPoint(poly, p3);
					index4 = mod->AddPoint(poly, p4);

					LONG padr[4] = { index1, index2, index3, index4 };
					LONG i = mod->CreateNgon(poly, padr, 4,MODELING_SETNGON_FLAG_FIXEDQUADS);
					if(!i) return TRUE;

					if(!mod->Commit(poly,MODELING_COMMIT_UPDATE)) return TRUE;

					index1 = (-index4)-1;
					index2 = (-index3)-1;

					hitPoint = newP;
				}
			}
		}
		DrawViews(DRAWFLAGS_ONLY_ACTIVE_VIEW|DRAWFLAGS_NO_THREAD|DRAWFLAGS_NO_ANIMATION);
	}
	win->MouseDragEnd();

	DrawViews(DRAWFLAGS_ONLY_ACTIVE_VIEW|DRAWFLAGS_NO_THREAD|DRAWFLAGS_NO_ANIMATION);
	return TRUE;
}

Bool RegisterSculptSnapTool()
{
	return RegisterToolPlugin(ID_SCULPSNAP_TOOL, GeLoadString(IDS_SCULPTSNAP_TOOL), 0, NULL, GeLoadString(IDS_SCULPTSNAP_TOOL), gNew SculptSnapTool);
}
