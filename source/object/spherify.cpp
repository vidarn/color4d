/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2011 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// deformer object example

#include "c4d.h"
#include "c4d_symbols.h"
#include "ospherifydeformer.h"

#define HANDLE_CNT 2

class Spherify : public ObjectData
{
	public:
		virtual Bool Init(GeListNode *node);

		virtual Bool Message				(GeListNode *node, LONG type, void *data);
		virtual void GetDimension		(BaseObject *op, Vector *mp, Vector *rad);
		virtual DRAWRESULT Draw			(BaseObject *op, DRAWPASS type, BaseDraw *bd, BaseDrawHelp *bh);
		virtual void GetHandle(BaseObject *op, LONG i, HandleInfo &info);
		virtual LONG DetectHandle		(BaseObject *op, BaseDraw *bd, LONG x, LONG y, QUALIFIER qualifier);
		virtual Bool MoveHandle			(BaseObject *op, BaseObject *undo, const Vector &mouse_pos, LONG hit_id, QUALIFIER qualifier, BaseDraw *bd);
		virtual Bool ModifyObject   (BaseObject *op, BaseDocument *doc, BaseObject *mod, const Matrix &op_mg, const Matrix &mod_mg, Real lod, LONG flags, BaseThread *thread);

		static NodeData *Alloc(void) { return gNew Spherify; }
};

Bool Spherify::Message(GeListNode *node, LONG type, void *data)
{
	if (type==MSG_MENUPREPARE)
	{
		((BaseObject*)node)->SetDeformMode(TRUE);
	}
	return TRUE;
}

Bool Spherify::ModifyObject(BaseObject *mod, BaseDocument *doc, BaseObject *op, const Matrix &op_mg, const Matrix &mod_mg, Real lod, LONG flags, BaseThread *thread)
{
	BaseContainer *data = mod->GetDataInstance(); 

	Vector  p,*padr=NULL;
	Matrix  m,im;
	LONG    i,pcnt;
	Real		rad=data->GetReal(SPHERIFYDEFORMER_RADIUS),strength=data->GetReal(SPHERIFYDEFORMER_STRENGTH);
	Real		s;
	SReal		*weight=NULL;

	if (!op->IsInstanceOf(Opoint)) return TRUE;
	
	padr = ToPoint(op)->GetPointW();
	pcnt = ToPoint(op)->GetPointCount(); if (!pcnt) return TRUE;

	weight = ToPoint(op)->CalcVertexMap(mod);

	m  = (!mod_mg) * op_mg; // op  ->  world  ->  modifier
	im = !m;

	for (i=0; i<pcnt; i++)
	{
		if (thread && !(i&63) && thread->TestBreak()) break;
		p = m*padr[i];
		s = strength;
		if (weight) s*=weight[i];
		p = s*(!p*rad)+(1.0-s)*p;
		padr[i] = p*im;
	}

	GeFree(weight);
	op->Message(MSG_UPDATE);
	
	return TRUE;
}

void Spherify::GetDimension(BaseObject *op, Vector *mp, Vector *rad)
{
	BaseContainer *data = op->GetDataInstance(); 
	*mp =0.0;
	*rad=data->GetReal(SPHERIFYDEFORMER_RADIUS);
}

DRAWRESULT Spherify::Draw(BaseObject *op, DRAWPASS drawpass, BaseDraw *bd, BaseDrawHelp *bh)
{
	if (drawpass==DRAWPASS_OBJECT)
	{ 
		BaseContainer *data = op->GetDataInstance(); 
		Real   rad=data->GetReal(SPHERIFYDEFORMER_RADIUS);
		Vector h;
		Matrix m = bh->GetMg();
		
		m.v1*=rad;
		m.v2*=rad;
		m.v3*=rad;
		
		bd->SetMatrix_Matrix(NULL, Matrix());
		bd->SetPen(bd->GetObjectColor(bh,op));
		bd->DrawCircle(m);
		h=m.v2; m.v2=m.v3; m.v3=h;
		bd->DrawCircle(m);
		h=m.v1; m.v1=m.v3; m.v3=h;
		bd->DrawCircle(m);
	}
	else if (drawpass==DRAWPASS_HANDLES)
	{
		LONG   i;
		LONG    hitid = op->GetHighlightHandle(bd);
		HandleInfo info;

		bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
		bd->SetMatrix_Matrix(op, bh->GetMg());
		for (i=0; i<HANDLE_CNT; i++)
		{
			GetHandle(op, i, info);
			if (hitid==i)
				bd->SetPen(GetViewColor(VIEWCOLOR_SELECTION_PREVIEW));
			else
				bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
			bd->DrawHandle(info.position,DRAWHANDLE_BIG, 0);
		}
		
		GetHandle(op, 1, info);
		bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
		bd->DrawLine(info.position, Vector(0.0), 0);
	}
	return DRAWRESULT_OK;
}

void Spherify::GetHandle(BaseObject *op, LONG i, HandleInfo &info)
{
	BaseContainer *data = op->GetDataInstance();
	if (!data) return;

	switch (i)
	{
	case 0: 
		info.position.x = data->GetReal(SPHERIFYDEFORMER_RADIUS);
		info.direction.x = 1.0;
		info.type = HANDLECONSTRAINTTYPE_LINEAR;
		break;
	case 1:
		info.position.x = data->GetReal(SPHERIFYDEFORMER_STRENGTH)*1000.0;
		info.direction.x = 1.0;
		info.type = HANDLECONSTRAINTTYPE_LINEAR;
	default: break;
	}
}

LONG Spherify::DetectHandle(BaseObject *op, BaseDraw *bd, LONG x, LONG y, QUALIFIER qualifier)
{
	if (qualifier&QUALIFIER_CTRL) return NOTOK;

	HandleInfo info;
	Matrix	mg = op->GetMg();
	LONG    i,ret=NOTOK;
	Vector	p;

	for (i=0; i<HANDLE_CNT; i++)
	{
		GetHandle(op, i, info);
		if (bd->PointInRange(info.position*mg,x,y)) 
		{
			ret=i;
			if (!(qualifier&QUALIFIER_SHIFT)) break;
		}
	}
	return ret;
}

Bool Spherify::MoveHandle(BaseObject *op, BaseObject *undo, const Vector &mouse_pos, LONG hit_id, QUALIFIER qualifier, BaseDraw *bd)
{
	BaseContainer *dst = op  ->GetDataInstance();

	HandleInfo info;
	
	Real val = mouse_pos.x;
	GetHandle(op, hit_id, info);

	if (bd)
	{
		Matrix mg = op->GetUpMg() * undo->GetMl();
		Vector pos = bd->ProjectPointOnLine(info.position * mg, info.direction ^ mg, mouse_pos.x, mouse_pos.y);
		val = (pos * !mg) * info.direction;
	}

	switch (hit_id)
	{
		case 0: 
			dst->SetReal(SPHERIFYDEFORMER_RADIUS,FCut(val,RCO 0.0,RCO MAXRANGE)); 
			break;
		
		case 1: 
			dst->SetReal(SPHERIFYDEFORMER_STRENGTH,FCut01(val*0.001)); 
			break;

		default:
			break;
	}
	return TRUE;
}

Bool Spherify::Init(GeListNode *node)
{	
	BaseObject		*op   = (BaseObject*)node;
	BaseContainer *data = op->GetDataInstance();

	data->SetReal(SPHERIFYDEFORMER_RADIUS,200.0);
	data->SetReal(SPHERIFYDEFORMER_STRENGTH,0.5);

	return TRUE;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_SPHERIFYOBJECT 1001158

Bool RegisterSpherify(void)
{
	return RegisterObjectPlugin(ID_SPHERIFYOBJECT,GeLoadString(IDS_SPHERIZE),OBJECT_MODIFIER,Spherify::Alloc,"Ospherifydeformer",AutoBitmap("spherify.tif"),0);
}
