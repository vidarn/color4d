#include "c4d.h"
#include "c4d_symbols.h"
#include "toolpickobjectsdk.h"
#include "lib_editortools.h"

#define ID_SAMPLE_PICK_OBJECT_TOOL			450000263

class PickObjectTool : public DescriptionToolData
{
public:
	virtual LONG				GetToolPluginId() { return ID_SAMPLE_PICK_OBJECT_TOOL; }
	virtual const String GetResourceSymbol() { return String("ToolPickObjectSDK"); }

	virtual void				InitDefaultSettings(BaseDocument *doc, BaseContainer &data);
	virtual Bool				MouseInput(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, EditorWindow *win, const BaseContainer &msg);
	virtual LONG				GetState(BaseDocument *doc);
	virtual Bool				GetCursorInfo(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, Real x, Real y, BaseContainer &bc);
	virtual TOOLDRAW		Draw(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, BaseDrawHelp *bh, BaseThread *bt,TOOLDRAWFLAGS flags);
	virtual Bool				GetDDescription(BaseDocument *doc, BaseContainer &data, Description *description,DESCFLAGS_DESC &flags);

private:
	Vector GetWorldCoordinates(BaseDraw* bd, const Matrix4 &m, Real x, Real y, Real z);

	BaseDraw* _lastBaseDraw;
	LONG _mouseX, _mouseY;
};

void PickObjectTool::InitDefaultSettings(BaseDocument *doc, BaseContainer &data)
{
	_lastBaseDraw = NULL;
	_mouseX = -1;
	_mouseY = -1;

	data.SetLong(MDATA_PICKOBJECT_MODE, MDATA_PICKOBJECT_MODE_CIRCLE);
	data.SetLong(MDATA_PICKOBJECT_CIRCLE_RAD, 40);
	data.SetLong(MDATA_PICKOBJECT_RECT_W, 50);
	data.SetLong(MDATA_PICKOBJECT_RECT_H, 30);

	DescriptionToolData::InitDefaultSettings(doc, data);
}

Bool PickObjectTool::MouseInput(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, EditorWindow *win, const BaseContainer &msg)
{
	LONG mode = data.GetLong(MDATA_PICKOBJECT_MODE);
	LONG x, y, l, xr = 0, yr = 0, wr = 0, hr = 0;
	Matrix4 m;
	ViewportPixel** pix = NULL;
	String str;
	char ch[200];
	Bool ret = FALSE;
	AutoAlloc <C4DObjectList> list;
	if (!list)
		return FALSE;

	VIEWPORT_PICK_FLAGS flags = VIEWPORT_PICK_FLAGS_ALLOW_OGL | VIEWPORT_PICK_FLAGS_USE_SEL_FILTER;
	if (data.GetBool(MDATA_PICKOBJECT_ONLY_VISIBLE))
		flags |= VIEWPORT_PICK_FLAGS_OGL_ONLY_VISIBLE;
	x = msg.GetLong(BFM_INPUT_X);
	y = msg.GetLong(BFM_INPUT_Y);
	LReal timer = 0.0;
	if (mode == MDATA_PICKOBJECT_MODE_CIRCLE)
	{
		LONG rad = data.GetLong(MDATA_PICKOBJECT_CIRCLE_RAD);
		timer = GeGetMilliSeconds();
		ret = ViewportSelect::PickObject(bd, doc, x, y, rad, xr, yr, wr, hr, pix, flags, NULL, list, &m);
		timer = GeGetMilliSeconds() - timer;
	}
	else if (mode == MDATA_PICKOBJECT_MODE_RECTANGLE)
	{
		LONG width = data.GetLong(MDATA_PICKOBJECT_RECT_W);
		LONG height = data.GetLong(MDATA_PICKOBJECT_RECT_H);
		x -= width / 2;
		y -= height / 2;
		timer = GeGetMilliSeconds();
		ret = ViewportSelect::PickObject(bd, doc, x, y, x + width, y + height, xr, yr, wr, hr, pix, flags, NULL, list, &m);
		timer = GeGetMilliSeconds() - timer;
	}
	if (ret)
	{
		sprintf(ch, "Picking region from (%d, %d), size (%d, %d)|", xr, yr, wr, hr);
		str += ch;
		for (l = 0; l < list->GetCount(); l++)
		{
			sprintf(ch, ", z = %.4f|", list->GetZ(l));
			str += "Found Object " + list->GetObject(l)->GetName() + ch;
		}
	}
	else
		str	= "PickObject failed";
	sprintf(ch, "|Time: %.2f us", float(timer) * 1000.0f);
	str += ch;

	GeFree(pix);
	GeOutString(str, GEMB_OK);

	return TRUE;
}

LONG PickObjectTool::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}

Vector PickObjectTool::GetWorldCoordinates(BaseDraw* bd, const Matrix4 &m, Real x, Real y, Real z)
{
	// pick object returns the view-projection matrix. This transforms a point in camera space into clip space.

	LONG l, t, r, b, w, h;
	Vector4 pos;
	Vector posWorld;

	bd->GetFrame(&l, &t, &r, &b);
	if (l == r || b == t)
		return Vector(0.0);

	w = r - l;
	h = b - t;

	// first, transform the points into clip space
	pos.x = (x - Real(l)) / Real(w);
	pos.y = (y - Real(t)) / Real(h);
	pos.z = z;
	pos.w = 1.0;
	pos = pos * 2.0f - Vector4(1.0f);
	pos.y = -pos.y;

	// apply the inverse view transform
	Matrix4 im = !m;
	pos = im * pos;
	pos.MakeVector3();

	// convert it into a 3-tupel
	posWorld = bd->GetMg() * GetVector3(pos);

	return posWorld;
}

Bool PickObjectTool::GetCursorInfo(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, Real x, Real y, BaseContainer &bc)
{
	if (bc.GetId() == BFM_CURSORINFO_REMOVE)
		_lastBaseDraw = NULL;
	else
	{
		_lastBaseDraw = bd;
		_mouseX = (LONG)x;
		_mouseY = (LONG)y;

		AutoAlloc <C4DObjectList> list;
		if (list)
		{
			// get the z position of the topmost object. The z range for objects is from -1 to 1.
			Real z = 1.0;
			String str;
			Matrix4 m;
			ViewportSelect::PickObject(bd, doc, _mouseX, _mouseY, 1, VIEWPORT_PICK_FLAGS_ALLOW_OGL | VIEWPORT_PICK_FLAGS_USE_SEL_FILTER | VIEWPORT_PICK_FLAGS_OGL_ONLY_TOPMOST, NULL, list, &m);
			if (list->GetCount() > 0)
				z = list->GetZ(0);
			if (z < 1.0)
			{
				Vector v = GetWorldCoordinates(bd, m, x, y, z);
				char ch[200];
				sprintf(ch, "Mouse coordinates: (%d, %d), world coordinates: (%.4f, %.4f, %.4f)", _mouseX, _mouseY, v.x, v.y, v.z);
				str = ch;
			}
			else
				str = "Mouse cursor is not over an object";
			StatusSetText(str);
		}
	}
	SpecialEventAdd(EVMSG_UPDATEHIGHLIGHT);
	return TRUE;
}

TOOLDRAW PickObjectTool::Draw(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, BaseDrawHelp *bh, BaseThread *bt,TOOLDRAWFLAGS flags)
{
	if ((flags & TOOLDRAWFLAGS_HIGHLIGHT) && _lastBaseDraw == bd)
	{
		LONG mode = data.GetLong(MDATA_PICKOBJECT_MODE);
		Vector col(1.0);
		bd->SetMatrix_Screen();
		bd->SetPen(col);
		if (mode == MDATA_PICKOBJECT_MODE_CIRCLE)
		{
			Matrix m;
			Real rad = (Real)data.GetLong(MDATA_PICKOBJECT_CIRCLE_RAD);
			m.off = Vector((Real)_mouseX, (Real)_mouseY, 0.0);
			m.v1 *= rad;
			m.v2 *= rad;
			bd->DrawCircle(m);
		}
		else if (mode == MDATA_PICKOBJECT_MODE_RECTANGLE)
		{
			LONG width = data.GetLong(MDATA_PICKOBJECT_RECT_W);
			LONG height = data.GetLong(MDATA_PICKOBJECT_RECT_H);
			LONG x1 = _mouseX - width / 2;
			LONG y1 = _mouseY - height / 2;
			LONG x2 = x1 + width;
			LONG y2 = y1 + height;
			bd->LineStripBegin();
			bd->LineStrip(Vector((Real)x1, (Real)y1, 0.0), col, 0);
			bd->LineStrip(Vector((Real)x2, (Real)y1, 0.0), col, 0);
			bd->LineStrip(Vector((Real)x2, (Real)y2, 0.0), col, 0);
			bd->LineStrip(Vector((Real)x1, (Real)y2, 0.0), col, 0);
			bd->LineStrip(Vector((Real)x1, (Real)y1, 0.0), col, 0);
			bd->LineStripEnd();
		}
		return TOOLDRAW_HIGHLIGHTS;
	}
	return TOOLDRAW_0;
}

Bool PickObjectTool::GetDDescription(BaseDocument *doc, BaseContainer &data, Description *description,DESCFLAGS_DESC &flags)
{
	Bool res = DescriptionToolData::GetDDescription(doc, data, description, flags);
	if (flags & DESCFLAGS_DESC_LOADED)
	{
		BaseContainer	*bc;

		LONG mode = data.GetLong(MDATA_PICKOBJECT_MODE);

		bc = description->GetParameterI(DescLevel(MDATA_PICKOBJECT_CIRCLE_RAD), NULL);
		if (bc) bc->SetLong(DESC_HIDE, mode != MDATA_PICKOBJECT_MODE_CIRCLE);

		bc = description->GetParameterI(DescLevel(MDATA_PICKOBJECT_RECT_W), NULL);
		if (bc) bc->SetLong(DESC_HIDE, mode == MDATA_PICKOBJECT_MODE_CIRCLE);
		bc = description->GetParameterI(DescLevel(MDATA_PICKOBJECT_RECT_H), NULL);
		if (bc) bc->SetLong(DESC_HIDE, mode == MDATA_PICKOBJECT_MODE_CIRCLE);
	}
	return res;
}

Bool RegisterPickObjectTool()
{
	return RegisterToolPlugin(ID_SAMPLE_PICK_OBJECT_TOOL, GeLoadString(IDS_PICKOBJECT_SDK), PLUGINFLAG_TOOL_OBJECTHIGHLIGHT, NULL, GeLoadString(IDS_HELP_PICKOBJECT_SDK), gNew PickObjectTool);
}
