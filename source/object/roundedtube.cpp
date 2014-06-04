/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// generator object example (with no input objects)

#include "c4d.h"
#include "c4d_symbols.h"
#include "oroundedtube.h"

class RoundedTube : public ObjectData
{
	public:
		String test;

		virtual Bool Init		(GeListNode *node);
		virtual Bool Read		(GeListNode *node, HyperFile *hf, LONG level);
		virtual Bool Write	(GeListNode *node, HyperFile *hf);

		virtual void GetDimension		(BaseObject *op, Vector *mp, Vector *rad);
		virtual DRAWRESULT Draw			(BaseObject *op, DRAWPASS type, BaseDraw *bd, BaseDrawHelp *bh);
		virtual LONG GetHandleCount(BaseObject *op);
		virtual void GetHandle(BaseObject *op, LONG i, HandleInfo &info);
		virtual void SetHandle(BaseObject *op, LONG i, Vector p, const HandleInfo &info);
		virtual BaseObject* GetVirtualObjects(BaseObject *op, HierarchyHelp *hh);
		virtual Bool Message(GeListNode *node, LONG type, void *t_data);

		static NodeData *Alloc(void) { return gNew RoundedTube; }
};

Bool RoundedTube::Message(GeListNode *node, LONG type, void *t_data)
{
	if (type==MSG_DESCRIPTION_VALIDATE)
	{
		BaseContainer *data = ((BaseObject*)node)->GetDataInstance();

		CutReal(*data,TUBEOBJECT_IRADX,0.0,data->GetReal(TUBEOBJECT_RAD));
		CutReal(*data,TUBEOBJECT_ROUNDRAD,0.0,data->GetReal(TUBEOBJECT_IRADX));
	}
	else if (type==MSG_MENUPREPARE)
	{
		((BaseObject*)node)->SetPhong(TRUE,FALSE,0.0);
	}
	return TRUE;
}

void RoundedTube::GetDimension(BaseObject *op, Vector *mp, Vector *rad)
{
	BaseContainer *data = op->GetDataInstance();

	Real rado,radx,rady;
	rado = data->GetReal(TUBEOBJECT_RAD);
	radx = data->GetReal(TUBEOBJECT_IRADX);
	rady = data->GetReal(TUBEOBJECT_IRADY);

	*mp =0.0;
	switch (data->GetLong(PRIM_AXIS))
	{
		case 0: case 1: *rad=Vector(rady,rado+radx,rado+radx); break;
		case 2: case 3: *rad=Vector(rado+radx,rady,rado+radx); break;
		case 4: case 5: *rad=Vector(rado+radx,rado+radx,rady); break;
	}
}

static BaseObject *GenerateLathe(Vector *cpadr, LONG cpcnt, LONG sub, BaseThread *bt)
{
  PolygonObject *op=NULL;
	UVWStruct	    us;
	UVWTag				*tag =NULL;
	Vector				*padr=NULL;
	CPolygon			*vadr=NULL;
	LONG					i,j,pcnt,vcnt,a,b,c,d;
	Real					len=0.0,sn,cs,v1,v2,*uvadr=NULL;
	UVWHandle			uvwptr;
	
	pcnt = cpcnt*sub;
	vcnt = cpcnt*sub;

	op=PolygonObject::Alloc(pcnt,vcnt);
	if (!op) goto Error;

	tag	= (UVWTag*)op->MakeVariableTag(Tuvw,vcnt);
	if (!tag) goto Error;

	padr=op->GetPointW();
	vadr=op->GetPolygonW();

	uvadr = GeAllocType(Real,cpcnt+1);
	if (!uvadr) goto Error;

	uvadr[0]=0.0;
	for (i=0; i<cpcnt; i++)
	{
		uvadr[i] = len;
		len+=Len(cpadr[(i+1)%cpcnt]-cpadr[i]);
	}

	if (len>0.0) len=1.0/len;
	for (i=0; i<cpcnt; i++)
		uvadr[i]*=len;

	uvadr[cpcnt]=1.0;

	vcnt=0;

	uvwptr = tag->GetDataAddressW();
	for (i=0; i<sub; i++)
	{
		SinCos(pi2*Real(i)/Real(sub),sn,cs);

		v1=Real(i  )/Real(sub);
		v2=Real(i+1)/Real(sub);

		if (bt && bt->TestBreak()) goto Error;

		for (j=0; j<cpcnt; j++)
		{
			a = cpcnt*i+j;
			padr[a] = Vector(cpadr[j].x*cs,cpadr[j].y,cpadr[j].x*sn);

			if (i<sub)
			{
				b = cpcnt*i          +((j+1)%cpcnt);
				c = cpcnt*((i+1)%sub)+((j+1)%cpcnt);
				d = cpcnt*((i+1)%sub)+j;

				us = UVWStruct(Vector(v1,1.0-uvadr[j],0.0),Vector(v1,1.0-uvadr[j+1],0.0),Vector(v2,1.0-uvadr[j+1],0.0),Vector(v2,1.0-uvadr[j],0.0));
				tag->Set(uvwptr,vcnt,us);

				vadr[vcnt++] = CPolygon(a,b,c,d);
			}
		}
	}

	GeFree(uvadr);

	op->Message(MSG_UPDATE);
	op->SetPhong(TRUE,TRUE,Rad(80.0));
	return op;

Error:
	GeFree(uvadr);
	blDelete(op);
	return NULL;
}

static LineObject *GenerateIsoLathe(Vector *cpadr, LONG cpcnt, LONG sub)
{
	LONG i;

  LineObject *op = LineObject::Alloc(cpcnt*4+sub*4,8); if (!op) return NULL;
	Segment *sadr = op->GetSegmentW();
	Vector  *padr=op->GetPointW();

	for (i=0; i<4; i++)
	{
		sadr[i].cnt    = cpcnt;
		sadr[i].closed = TRUE;
	}
	for (i=0; i<4; i++)
	{
		sadr[4+i].cnt    = sub;
		sadr[4+i].closed = TRUE;
	}

	Real	sn,cs;
	LONG	j;

	for (i=0; i<4; i++)
	{
		SinCos(Real(i)*pi05,sn,cs);
		for (j=0; j<cpcnt; j++)
			padr[i*cpcnt+j] = Vector(cpadr[j].x*cs,cpadr[j].y,cpadr[j].x*sn);
	}

	for (i=0; i<sub; i++)
	{
		SinCos(Real(i)/sub*pi2,sn,cs);
		for (j=0; j<4; j++)
			padr[4*cpcnt+j*sub+i] = Vector(cpadr[cpcnt/4*j].x*cs,cpadr[cpcnt/4*j].y,cpadr[cpcnt/4*j].x*sn);
	}

	op->Message(MSG_UPDATE);
	return op;
}

Bool RoundedTube::Init(GeListNode *node)
{
	test = String("Test");

	BaseObject		*op   = (BaseObject*)node;
	BaseContainer *data = op->GetDataInstance();

	data->SetReal(TUBEOBJECT_RAD,200.0);
	data->SetReal(TUBEOBJECT_IRADX,50.0);
	data->SetReal(TUBEOBJECT_IRADY,50.0);
	data->SetLong(TUBEOBJECT_SUB,1);
	data->SetLong(TUBEOBJECT_ROUNDSUB,8);
	data->SetReal(TUBEOBJECT_ROUNDRAD,10.0);
	data->SetLong(TUBEOBJECT_SEG,36);
	data->SetLong(PRIM_AXIS,PRIM_AXIS_YP);

	return TRUE;
}

Bool RoundedTube::Read(GeListNode *node, HyperFile *hf, LONG level)
{
	if (level>=0)
	{
		hf->ReadString(&test);
	}
	return TRUE;
}

Bool RoundedTube::Write(GeListNode *node, HyperFile *hf)
{
	hf->WriteString(test);
	return TRUE;
}

static void SetAxis(BaseObject *obj, LONG axis)
{
	PointObject *op = ToPoint(obj);
	if (axis==2) return;

	Vector *padr = op->GetPointW();
	LONG pcnt  = op->GetPointCount(),i;

	switch (axis)
	{
		case 0: // +X
			for (i=0; i<pcnt; i++)
				padr[i] = Vector(padr[i].y,-padr[i].x,padr[i].z);
			break;

		case 1: // -X
			for (i=0; i<pcnt; i++)
				padr[i] = Vector(-padr[i].y,padr[i].x,padr[i].z);
			break;

		case 3: // -Y
			for (i=0; i<pcnt; i++)
				padr[i] = Vector(-padr[i].x,-padr[i].y,padr[i].z);
			break;

		case 4: // +Z
			for (i=0; i<pcnt; i++)
				padr[i] = Vector(padr[i].x,-padr[i].z,padr[i].y);
			break;

		case 5: // -Z
			for (i=0; i<pcnt; i++)
				padr[i] = Vector(padr[i].x,padr[i].z,-padr[i].y);
			break;
	}

	op->Message(MSG_UPDATE);
}

BaseObject *RoundedTube::GetVirtualObjects(BaseObject *op, HierarchyHelp *hh)
{
	LineObject *lop = NULL;
	BaseObject *ret = NULL;

	Bool dirty = op->CheckCache(hh) || op->IsDirty(DIRTYFLAGS_DATA);
	if (!dirty) return op->GetCache(hh);

	BaseContainer *data = op->GetDataInstance();

	Real rad	  = data->GetReal(TUBEOBJECT_RAD,200.0);
	Real iradx  = data->GetReal(TUBEOBJECT_IRADX,50.0);
	Real irady  = data->GetReal(TUBEOBJECT_IRADY,50.0);
	Real rrad   = data->GetReal(TUBEOBJECT_ROUNDRAD,10.0);
	LONG sub    = CalcLOD(data->GetLong(TUBEOBJECT_SUB,1),hh->GetLOD(),1,1000);
	LONG rsub   = CalcLOD(data->GetLong(TUBEOBJECT_ROUNDSUB,8),hh->GetLOD(),1,1000);
	LONG seg		= CalcLOD(data->GetLong(TUBEOBJECT_SEG,36),hh->GetLOD(),3,1000);
	LONG axis   = data->GetLong(PRIM_AXIS);
	LONG i;
	Real sn,cs;

	LONG cpcnt  = 4*(sub+rsub);
	Vector *cpadr = GeAllocType(Vector,cpcnt);
	if (!cpadr) return NULL;

	for (i=0; i<sub; i++)
	{
		cpadr[i             ]=Vector(rad-iradx,(1.0-Real(i)/Real(sub)*2.0)*(irady-rrad),0.0);
		cpadr[i+   sub+rsub ]=Vector(rad+(Real(i)/Real(sub)*2.0-1.0)*(iradx-rrad),-irady,0.0);
		cpadr[i+2*(sub+rsub)]=Vector(rad+iradx,(Real(i)/Real(sub)*2.0-1.0)*(irady-rrad),0.0);
		cpadr[i+3*(sub+rsub)]=Vector(rad+(1.0-Real(i)/Real(sub)*2.0)*(iradx-rrad),irady,0.0);
	}
	for (i=0; i<rsub; i++)
	{
		SinCos(Real(i)/Real(rsub)*pi05,sn,cs);
		cpadr[i+sub             ]=Vector(rad-(iradx-rrad+cs*rrad),-(irady-rrad+sn*rrad),0.0);
		cpadr[i+sub+  (sub+rsub)]=Vector(rad+(iradx-rrad+sn*rrad),-(irady-rrad+cs*rrad),0.0);
		cpadr[i+sub+2*(sub+rsub)]=Vector(rad+(iradx-rrad+cs*rrad),+(irady-rrad+sn*rrad),0.0);
		cpadr[i+sub+3*(sub+rsub)]=Vector(rad-(iradx-rrad+sn*rrad),+(irady-rrad+cs*rrad),0.0);
	}

	ret = GenerateLathe(cpadr,cpcnt,seg,hh->GetThread());
	if (!ret) goto Error;
	SetAxis(ret,axis);
	ret->KillTag(Tphong);
	if (!op->CopyTagsTo(ret,TRUE,FALSE,FALSE,NULL)) goto Error;

	ret->SetName(op->GetName());

	if (hh->GetBuildFlags()&BUILDFLAGS_ISOPARM)
	{
		lop = GenerateIsoLathe(cpadr,cpcnt,seg);
		if (!lop) goto Error;
		SetAxis(lop,axis);
		ret->SetIsoparm(lop);
	}

	GeFree(cpadr);
	return ret;

Error:
	GeFree(cpadr);
	blDelete(ret);
	return NULL;
}

static Vector SwapPoint(const Vector &p, LONG axis)
{
	switch (axis)
	{
		case 0: return Vector(p.y,-p.x,p.z); break;
		case 1: return Vector(-p.y,p.x,p.z); break;
		case 3: return Vector(-p.x,-p.y,p.z); break;
		case 4: return Vector(p.x,-p.z,p.y); break;
		case 5: return Vector(p.x,p.z,-p.y); break;
	}
	return p;
}

LONG RoundedTube::GetHandleCount(BaseObject *op)
{
	return 5;
}
void RoundedTube::GetHandle(BaseObject *op, LONG id, HandleInfo &info)
{
	BaseContainer *data = op->GetDataInstance();

	Real		rad	   = data->GetReal(TUBEOBJECT_RAD);
	Real		iradx  = data->GetReal(TUBEOBJECT_IRADX);
	Real		irady  = data->GetReal(TUBEOBJECT_IRADY);
	Real		rrad   = data->GetReal(TUBEOBJECT_ROUNDRAD);
	LONG		axis   = data->GetLong(PRIM_AXIS);

	switch (id)
	{
	case 0: info.position = Vector(rad,0.0,0.0);							info.direction = Vector(1.0, 0.0, 0.0); break;
	case 1: info.position = Vector(rad+iradx,0.0,0.0);				info.direction = Vector(1.0, 0.0, 0.0); break;
	case 2: info.position = Vector(rad,irady,0.0);						info.direction = Vector(0.0, 1.0, 0.0); break;
	case 3: info.position = Vector(rad+iradx,irady-rrad,0.0); info.direction = Vector(0.0, -1.0, 0.0); break;
	case 4: info.position = Vector(rad+iradx-rrad,irady,0.0); info.direction = Vector(-1.0, 0.0, 0.0); break;
	}

	info.type = HANDLECONSTRAINTTYPE_LINEAR;
	info.position = SwapPoint(info.position ,axis);
	info.direction = SwapPoint(info.direction ,axis);
}

void RoundedTube::SetHandle(BaseObject *op, LONG i, Vector p, const HandleInfo &info)
{
	BaseContainer *data = op->GetDataInstance();
	if (!data) return;

	HandleInfo inf;
	GetHandle(op, i, inf);

	Real val = (p - inf.position) * info.direction;

	switch (i)
	{
	case 0: data->SetReal(TUBEOBJECT_RAD,FCut(data->GetReal(TUBEOBJECT_RAD)+val,data->GetReal(TUBEOBJECT_IRADX),RCO MAXRANGE)); break;
	case 1: data->SetReal(TUBEOBJECT_IRADX,FCut(data->GetReal(TUBEOBJECT_IRADX)+val,data->GetReal(TUBEOBJECT_ROUNDRAD),data->GetReal(TUBEOBJECT_RAD))); break;
	case 2: data->SetReal(TUBEOBJECT_IRADY,FCut(data->GetReal(TUBEOBJECT_IRADY)+val,data->GetReal(TUBEOBJECT_ROUNDRAD),RCO MAXRANGE)); break;
	case 3: 
	case 4: data->SetReal(TUBEOBJECT_ROUNDRAD,FCut(data->GetReal(TUBEOBJECT_ROUNDRAD)+val,RCO 0.0,FMin(data->GetReal(TUBEOBJECT_IRADX),data->GetReal(TUBEOBJECT_IRADY)))); break;
	default: break;
	}
}

DRAWRESULT RoundedTube::Draw(BaseObject *op, DRAWPASS drawpass, BaseDraw *bd, BaseDrawHelp *bh)
{
	if (drawpass!=DRAWPASS_HANDLES) return DRAWRESULT_SKIP;

	BaseContainer *data = op->GetDataInstance();

	LONG    i;
	Real		rad	   = data->GetReal(TUBEOBJECT_RAD);
	Real		iradx  = data->GetReal(TUBEOBJECT_IRADX);
	Real		irady  = data->GetReal(TUBEOBJECT_IRADY);
	LONG		axis   = data->GetLong(PRIM_AXIS);

	bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));

	HandleInfo info;
	LONG hitid = op->GetHighlightHandle(bd);
	bd->SetMatrix_Matrix(op, bh->GetMg());

	for (i = GetHandleCount(op) - 1; i >= 0; --i)
	{
		GetHandle(op, i, info);

		if (i==hitid)
			bd->SetPen(GetViewColor(VIEWCOLOR_SELECTION_PREVIEW));
		else
			bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
		bd->DrawHandle(info.position,DRAWHANDLE_BIG, 0);

		//Draw lines to the handles
		bd->SetPen(GetViewColor(VIEWCOLOR_ACTIVEPOINT));
		switch (i)
		{
		case 0:
			{
				HandleInfo p2;
				GetHandle(op, 1, p2);
				bd->DrawLine(info.position, p2.position,0);
				GetHandle(op, 2, p2);
				bd->DrawLine(info.position, p2.position,0);
			}
			break;
		case 3:
			bd->DrawLine(info.position,SwapPoint(Vector(rad+iradx,irady,0.0),axis),0);
			break;
		case 4:
			bd->DrawLine(info.position,SwapPoint(Vector(rad+iradx,irady,0.0),axis),0);
			break;
		default: break;
		}
	}

	return DRAWRESULT_OK;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_ROUNDEDTUBEOBJECT 1001157

Bool RegisterRoundedTube(void)
{
	return RegisterObjectPlugin(ID_ROUNDEDTUBEOBJECT,GeLoadString(IDS_ROUNDED_TUBE),OBJECT_GENERATOR,RoundedTube::Alloc,"Oroundedtube",AutoBitmap("roundedtube.tif"),0);
}
