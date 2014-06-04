/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code for a metaball painting tool

#include "c4d.h"
#include "c4d_symbols.h"

#define ID_LIQUIDTOOL 1000973

class LiquidToolData : public ToolData
{
	public:
		virtual Bool			MouseInput(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, EditorWindow *win, const BaseContainer &msg);
		virtual Bool			KeyboardInput(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, EditorWindow *win, const BaseContainer &msg);
		virtual LONG			GetState(BaseDocument *doc);
		virtual Bool      GetCursorInfo(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, Real x, Real y, BaseContainer &bc);
		virtual TOOLDRAW  Draw(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, BaseDrawHelp *bh, BaseThread *bt,TOOLDRAWFLAGS flags);

		virtual SubDialog*	AllocSubDialog(BaseContainer* bc);
};

LONG LiquidToolData::GetState(BaseDocument *doc)
{
	if (doc->GetMode()==Mpaint) return 0;
	return CMD_ENABLED;
}

Bool LiquidToolData::KeyboardInput(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, EditorWindow *win, const BaseContainer &msg)
{
	LONG key = msg.GetData(BFM_INPUT_CHANNEL).GetLong();
	String str = msg.GetData(BFM_INPUT_ASC).GetString();
	if (key == KEY_ESC)
	{
		// do what you want

		// return TRUE to signal that the key is processed!
		return TRUE;
	}
	return FALSE;
}

Bool LiquidToolData::MouseInput(BaseDocument *doc, BaseContainer &data, BaseDraw *bd,EditorWindow *win, const BaseContainer &msg)
{
	Real mx = msg.GetReal(BFM_INPUT_X);
	Real my = msg.GetReal(BFM_INPUT_Y);
	LONG button;
	
	switch (msg.GetLong(BFM_INPUT_CHANNEL))
	{
		case BFM_INPUT_MOUSELEFT : button=KEY_MLEFT; break;
		case BFM_INPUT_MOUSERIGHT: button=KEY_MRIGHT; break;
		default: return TRUE;
	}
	
	BaseObject	*cl=NULL,*null=NULL,*op=NULL;
	Real				dx,dy,rad=5.0;
	Bool				newmeta=FALSE;

	op = BaseObject::Alloc(Osphere);
	if (!op) return FALSE;

	null = BaseObject::Alloc(Ometaball);
	{
		null->GetDataInstance()->SetReal(METABALLOBJECT_SUBEDITOR,10.0);
		null->MakeTag(Tphong);
	}
	newmeta=TRUE;

	doc->AddUndo(UNDOTYPE_NEW,null);

	if (newmeta)
	{
		doc->InsertObject(null,NULL,NULL);
		doc->SetActiveObject(null);
		DrawViews(DRAWFLAGS_ONLY_ACTIVE_VIEW|DRAWFLAGS_NO_THREAD|DRAWFLAGS_NO_ANIMATION);
	}

	BaseContainer bc;
	BaseContainer device;
	win->MouseDragStart(button,mx,my,MOUSEDRAGFLAGS_DONTHIDEMOUSE|MOUSEDRAGFLAGS_NOMOVE);
	while (win->MouseDrag(&dx,&dy,&device)==MOUSEDRAGRESULT_CONTINUE)
	{
		bc=BaseContainer();
		win->BfGetInputEvent(BFM_INPUT_MOUSE,&bc);
		if (bc.GetLong(BFM_INPUT_CHANNEL)==BFM_INPUT_MOUSEWHEEL)
		{
			rad+=bc.GetReal(BFM_INPUT_VALUE)/120.0;
			rad=FCut(rad,RCO 0.1,RCO MAXRANGE);
			GePrint(RealToString(rad));
		}

		if (dx==0.0 && dy==0.0) continue;

		mx+=dx;
		my+=dy;
		cl=(BaseObject*)op->GetClone(COPYFLAGS_0,NULL);
		if (!cl) break;
		
		cl->GetDataInstance()->SetReal(PRIM_SPHERE_RAD,rad);

		cl->SetAbsPos(bd->SW(Vector(mx,my,500.0)));
		cl->InsertUnder(null);
		DrawViews(DRAWFLAGS_ONLY_ACTIVE_VIEW|DRAWFLAGS_NO_THREAD|DRAWFLAGS_NO_ANIMATION);
	}

	if (win->MouseDragEnd()==MOUSEDRAGRESULT_ESCAPE)
	{
		doc->DoUndo(TRUE);
	}

	BaseObject::Free(op);

	EventAdd();
	return TRUE;
}

Bool LiquidToolData::GetCursorInfo(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, Real x,Real y,BaseContainer &bc)
{
	if (bc.GetId()==BFM_CURSORINFO_REMOVE) 
		return TRUE;

	bc.SetString(RESULT_BUBBLEHELP,GeLoadString(IDS_PRIMITIVETOOL));
	bc.SetLong(RESULT_CURSOR,MOUSE_POINT_HAND);
	return TRUE;
}

TOOLDRAW LiquidToolData::Draw(BaseDocument *doc, BaseContainer &data, BaseDraw *bd, BaseDrawHelp *bh, BaseThread *bt,TOOLDRAWFLAGS flags)
{
	bd->SetMatrix_Matrix(NULL, Matrix());
	if (flags & TOOLDRAWFLAGS_HIGHLIGHT)
	{
		// Draw your stuff inside the highlight plane
		Vector p[3] = { Vector(0,0,0),Vector(100,0,0),Vector(50,100,0)};
		Vector f[3] = { Vector(1,0,0),Vector(1,0,0),Vector(1,0,0)};
		bd->DrawPolygon(p,f,FALSE);
	}
	else if (flags & TOOLDRAWFLAGS_INVERSE_Z)
	{
		// Draw your stuff into the active plane - invisible Z
		Vector p[3] = { Vector(0,0,0),Vector(100,0,0),Vector(50,-100,0)};
		Vector f[3] = { Vector(0,0,1),Vector(0,0,1),Vector(0,0,1)};
		bd->DrawPolygon(p,f,FALSE);
	}
	else if (!flags)
	{
		// Draw your stuff into the active plane - visible Z
		Vector p[3] = { Vector(0,0,0),Vector(-100,0,0),Vector(-50,100,0)};
		Vector f[3] = { Vector(0,1,0),Vector(0,1,0),Vector(0,1,0)};
		bd->DrawPolygon(p,f,FALSE);
	}

	return TOOLDRAW_HANDLES|TOOLDRAW_AXIS;
}

class LiquidToolDialog: public SubDialog
{
	public:
		virtual Bool CreateLayout(void);
		virtual Bool InitValues(void);

		virtual Bool InitDialog(void);
		virtual Bool Command(LONG id,const BaseContainer &msg);
};

Bool LiquidToolDialog::CreateLayout(void)
{
	GroupBegin(0,BFH_SCALEFIT,1,0,"",0);
		GroupBegin(0,BFH_SCALEFIT,2,0,"",0);
			GroupSpace(4,1);

			AddStaticText(0,0,0,0,"R",0);
			AddEditSlider(1000,BFH_SCALEFIT);

			AddStaticText(0,0,0,0,"G",0);
			AddEditSlider(1001,BFH_SCALEFIT);

			AddStaticText(0,0,0,0,"B",0);
			AddEditSlider(1002,BFH_SCALEFIT);
		GroupEnd();

	GroupEnd();
	return TRUE;
}

Bool LiquidToolDialog::InitValues(void)
{
	return InitDialog();
}

Bool LiquidToolDialog::InitDialog(void)
{
	BaseContainer *bc=GetToolData(GetActiveDocument(),ID_LIQUIDTOOL);
	if (!bc) return FALSE;

	return TRUE;
}

Bool LiquidToolDialog::Command(LONG id,const BaseContainer &msg)
{
	return TRUE;
}

SubDialog* LiquidToolData::AllocSubDialog(BaseContainer* bc)
{
	return gNew LiquidToolDialog; 
}

Bool RegisterPrimitiveTool(void)
{
	return RegisterToolPlugin(ID_LIQUIDTOOL,GeLoadString(IDS_PRIMITIVETOOL),0,AutoBitmap("liquid.tif"),"C++ SDK Liquid Painting Tool",gNew LiquidToolData);
}
