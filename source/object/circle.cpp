/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2011 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// spline example 

#include "c4d.h"
#include "c4d_symbols.h"
#include "odoublecircle.h"

class DoubleCircleData : public ObjectData
{
	INSTANCEOF(DoubleCircleData,ObjectData)

	public:
		virtual Bool Init(GeListNode *node);

		virtual Bool Message				(GeListNode *node, LONG type, void *data);
		virtual LONG GetHandleCount(BaseObject *op);
		virtual void GetHandle(BaseObject *op, LONG i, HandleInfo &info);
		virtual void SetHandle(BaseObject *op, LONG i, Vector p, const HandleInfo &info);
		virtual SplineObject* GetContour(BaseObject *op, BaseDocument *doc, Real lod, BaseThread *bt);
		virtual Bool GetDEnabling(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc);

		static NodeData *Alloc(void) { return gNew DoubleCircleData; }
};

Bool DoubleCircleData::Message(GeListNode *node, LONG type, void *data)
{
	if (type==MSG_MENUPREPARE)
	{
		BaseDocument *doc = (BaseDocument*)data;
		((BaseObject*)node)->GetDataInstance()->SetLong(PRIM_PLANE,doc->GetSplinePlane());
	}

	return TRUE;
}

Bool DoubleCircleData::Init(GeListNode *node)
{	
	BaseObject		*op   = (BaseObject*)node;
	BaseContainer *data = op->GetDataInstance();
	if (!data) return FALSE;

	data->SetReal(CIRCLEOBJECT_RAD,200.0);
	data->SetLong(PRIM_PLANE,0);
	data->SetBool(PRIM_REVERSE,FALSE);
	data->SetLong(SPLINEOBJECT_INTERPOLATION,SPLINEOBJECT_INTERPOLATION_ADAPTIVE);
	data->SetLong(SPLINEOBJECT_SUB,8);
	data->SetReal(SPLINEOBJECT_ANGLE,Rad(5.0));
	data->SetReal(SPLINEOBJECT_MAXIMUMLENGTH,5.0);

	return TRUE;
}

static Vector SwapPoint(const Vector &p, LONG plane)
{
	switch (plane)
	{
		case 1: return Vector(-p.z,p.y,p.x); break;
		case 2: return Vector(p.x,-p.z,p.y); break;
	}
	return p;
}

LONG DoubleCircleData::GetHandleCount(BaseObject *op)
{
	return 1;
}
void DoubleCircleData::GetHandle(BaseObject *op, LONG i, HandleInfo &info)
{
	BaseContainer *data = op->GetDataInstance();
	if (!data) return;

	Real rad	  = data->GetReal(CIRCLEOBJECT_RAD);
	LONG plane  = data->GetLong(PRIM_PLANE);

	info.position = SwapPoint(Vector(rad,0.0,0.0),plane);
	info.direction = !SwapPoint(Vector(1.0,0.0,0.0),plane);
	info.type = HANDLECONSTRAINTTYPE_LINEAR;
}

void DoubleCircleData::SetHandle(BaseObject *op, LONG i, Vector p, const HandleInfo &info)
{
	BaseContainer *data = op->GetDataInstance();
	if (!data) return;

	Real val = p * info.direction;

	data->SetReal(CIRCLEOBJECT_RAD,FCut(val,RCO 0.0,RCO MAXRANGE));
}

SplineObject *GenerateCircle(Real rad)
{
	#define TANG 0.415

	Real	sn,cs;
	LONG	i,sub=4;

	SplineObject *op = SplineObject::Alloc(sub*2,SPLINETYPE_BEZIER);
	if (!op || !op->MakeVariableTag(Tsegment,2)) { blDelete(op); return NULL; }
  op->GetDataInstance()->SetBool(SPLINEOBJECT_CLOSED,TRUE);

	Vector  *padr = op->GetPointW();
	Tangent *hadr = op->GetTangentW();
	Segment *sadr = op->GetSegmentW();

	if (sadr)
	{
		sadr[0].closed = TRUE;
		sadr[0].cnt    = sub;
		sadr[1].closed = TRUE;
		sadr[1].cnt    = sub;
	}

	if (hadr && padr)
	{
		for (i=0; i<sub; i++)
		{
			SinCos(2.0*pi*i/Real(sub),sn,cs);
			
			padr[i]    = Vector(cs*rad,sn*rad,0.0);
			hadr[i].vl = Vector(sn*rad*TANG,-cs*rad*TANG,0.0);
			hadr[i].vr = -hadr[i].vl;

			padr[i+sub]    = Vector(cs*rad,sn*rad,0.0)*0.5;
			hadr[i+sub].vl = Vector(sn*rad*TANG,-cs*rad*TANG,0.0)*0.5;
			hadr[i+sub].vr = -hadr[i+sub].vl;
		}
	}

	op->Message(MSG_UPDATE);

	return op;
}

static void OrientObject(SplineObject *op, LONG plane, Bool reverse)
{
	Vector  *padr=ToPoint(op)->GetPointW();
	Tangent *hadr=ToSpline(op)->GetTangentW(),h;
	LONG		pcnt =ToPoint(op)->GetPointCount(),i;

	if (!hadr && ToSpline(op)->GetTangentCount()) return;
	if (!padr && ToPoint(op)->GetPointCount()) return;

	if (plane>=1)
	{
		switch (plane)
		{
			case 1: // ZY
				for (i=0; i<pcnt; i++)
				{
					padr[i]    = Vector(-padr[i].z,padr[i].y,padr[i].x);
					if (!hadr) continue;
					hadr[i].vl = Vector(-hadr[i].vl.z,hadr[i].vl.y,hadr[i].vl.x);
					hadr[i].vr = Vector(-hadr[i].vr.z,hadr[i].vr.y,hadr[i].vr.x);
				}
				break;

			case 2: // XZ
				for (i=0; i<pcnt; i++)
				{
					padr[i] = Vector(padr[i].x,-padr[i].z,padr[i].y);
					if (!hadr) continue;
					hadr[i].vl = Vector(hadr[i].vl.x,-hadr[i].vl.z,hadr[i].vl.y);
					hadr[i].vr = Vector(hadr[i].vr.x,-hadr[i].vr.z,hadr[i].vr.y);
				}
				break;
		}
	}
	
	if (reverse)
	{
		Vector	p;
		LONG		to=pcnt/2;
		if (pcnt%2) to++;
		for (i=0; i<to; i++)
		{
			p=padr[i]; padr[i]=padr[pcnt-1-i]; padr[pcnt-1-i]=p;
			if (!hadr) continue;
			h=hadr[i]; 
			hadr[i].vl=hadr[pcnt-1-i].vr; 
			hadr[i].vr=hadr[pcnt-1-i].vl; 
			hadr[pcnt-1-i].vl=h.vr;
			hadr[pcnt-1-i].vr=h.vl;
		}
	}
	op->Message(MSG_UPDATE);
}

SplineObject *DoubleCircleData::GetContour(BaseObject *op, BaseDocument *doc, Real lod, BaseThread *bt)
{
	BaseContainer *bc=op->GetDataInstance();
	if (!bc) return NULL;
	SplineObject *bp = GenerateCircle(bc->GetReal(CIRCLEOBJECT_RAD));
	if (!bp) return NULL;
	BaseContainer *bb=bp->GetDataInstance();

	bb->SetLong(SPLINEOBJECT_INTERPOLATION,bc->GetLong(SPLINEOBJECT_INTERPOLATION));
	bb->SetLong(SPLINEOBJECT_SUB,bc->GetLong(SPLINEOBJECT_SUB));
	bb->SetReal(SPLINEOBJECT_ANGLE,bc->GetReal(SPLINEOBJECT_ANGLE));
	bb->SetReal(SPLINEOBJECT_MAXIMUMLENGTH,bc->GetReal(SPLINEOBJECT_MAXIMUMLENGTH));

	OrientObject(bp,bc->GetLong(PRIM_PLANE),bc->GetBool(PRIM_REVERSE));

	return bp;
}

Bool DoubleCircleData::GetDEnabling(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc)
{
	LONG inter;
	BaseContainer *data = ((BaseObject*)node)->GetDataInstance();
	if (!data) return FALSE;

	switch (id[0].id)
	{
		case SPLINEOBJECT_SUB:		
			inter=data->GetLong(SPLINEOBJECT_INTERPOLATION);
			return inter==SPLINEOBJECT_INTERPOLATION_NATURAL || inter==SPLINEOBJECT_INTERPOLATION_UNIFORM;

		case SPLINEOBJECT_ANGLE:	
			inter = data->GetLong(SPLINEOBJECT_INTERPOLATION);
			return inter==SPLINEOBJECT_INTERPOLATION_ADAPTIVE || inter==SPLINEOBJECT_INTERPOLATION_SUBDIV;

		case SPLINEOBJECT_MAXIMUMLENGTH:	
			return data->GetLong(SPLINEOBJECT_INTERPOLATION)==SPLINEOBJECT_INTERPOLATION_SUBDIV;
	}
	return TRUE;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_CIRCLEOBJECT 1001154

Bool RegisterCircle(void)
{
	return RegisterObjectPlugin(ID_CIRCLEOBJECT,GeLoadString(IDS_CIRCLE),OBJECT_GENERATOR|OBJECT_ISSPLINE,DoubleCircleData::Alloc,"Odoublecircle",AutoBitmap("circle.tif"),0);
}
