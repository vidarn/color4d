/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2007 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code for creating a collision object for Hair

//////////////////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_hair.h"

#include "ohairsdkcollider.h"

//////////////////////////////////////////////////////////////////////////

class HairCollisionObject : public ObjectData
{
	INSTANCEOF(HairCollisionObject,ObjectData)

public:

	virtual Bool Init(GeListNode *node);
	virtual void Free(GeListNode *node);

	virtual Bool Message(GeListNode *node, LONG type, void *data);
	virtual DRAWRESULT Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh);

	static NodeData *Alloc(void) { return gNew HairCollisionObject; }

	//////////////////////////////////////////////////////////////////////////
	
	HairPluginObjectData m_FnTable;
};

//////////////////////////////////////////////////////////////////////////

inline LONG Sgn(Real r) { return (r<0.0)?-1:1; }

static Bool _CollisionFn(BaseDocument *doc, BaseList2D *op, HairObject *hair, HairGuides *guides, HairGuideDynamics *dyn, const Vector &bmin, const Vector &bmax, Real t1, Real t2, Real pr, Vector *oldpnt, Vector *newpnt, Vector *vel, Real *invmass, LONG pcnt, LONG cnt, LONG scnt)
{
	BaseContainer *bc=op->GetDataInstance();

	Real bounce=-bc->GetReal(HAIRSDK_COLLIDER_BOUNCE);
	Real friction=1.0-bc->GetReal(HAIRSDK_COLLIDER_FRICTION);
	Real width=bc->GetReal(HAIRSDK_COLLIDER_WIDTH);
	Real height=bc->GetReal(HAIRSDK_COLLIDER_HEIGHT);

	LONG i,l,j;

	Matrix mg=((BaseObject*)op)->GetMg(),mi=!mg;

	for (i=0;i<cnt;i++)
	{
		for (l=0;l<scnt;l++)
		{
			j=i*scnt+l;

			if (invmass[j]==0.0) continue;

			Vector np=newpnt[j]*mi,op=oldpnt[j]*mi;
			Vector v=vel[j]^mi;
			Real nz,oz;

			if (Abs(op.z)<=pr)
			{
				if (op.z<0.0) op.z=-pr-1e-4;
				else op.z=pr+1e-4;
			}

			if (op.z<0.0) { oz=op.z+pr; nz=np.z+pr; }
			else { oz=op.z-pr; nz=np.z-pr; }

			if (Sgn(oz)==Sgn(nz) && Abs(oz)>1e-4 && Abs(nz)>1e-4) continue;

			Real zdlt=np.z-op.z;
			if (zdlt!=0.0) zdlt=Abs(oz/zdlt);

			Vector dv=np-op;

			op=op+zdlt*dv;
			op.z-=1e-4;

			if (Abs(op.x)>width || Abs(op.y)>height) continue;
			
			v.z*=bounce;
			v.x*=friction;
			v.y*=friction;

			dv.z*=bounce;
			dv.x*=friction;
			dv.y*=friction;
			
			np=op+dv*(1.0-zdlt);

			newpnt[j]=np*mg;
			oldpnt[j]=op*mg;
			vel[j]=v^mg;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

Bool HairCollisionObject::Init(GeListNode *node)
{
	BaseContainer *bc=((BaseList2D*)node)->GetDataInstance();
	
	bc->SetReal(HAIRSDK_COLLIDER_BOUNCE,0.3);
	bc->SetReal(HAIRSDK_COLLIDER_FRICTION,0.1);
	bc->SetReal(HAIRSDK_COLLIDER_WIDTH,200.0);
	bc->SetReal(HAIRSDK_COLLIDER_HEIGHT,200.0);

	m_FnTable.calc_collision=_CollisionFn;

	return TRUE;
}

void HairCollisionObject::Free(GeListNode *node)
{
}

Bool HairCollisionObject::Message(GeListNode *node, LONG type, void *data)
{
	if (type==MSG_HAIR_GET_OBJECT_TYPE && data)
	{
		HairPluginMessageData *mdata=(HairPluginMessageData*)data;
		mdata->data=&m_FnTable;
		return TRUE;
	}
	
	return SUPER::Message(node,type,data);
}

DRAWRESULT HairCollisionObject::Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh)
{
	if (drawpass!=DRAWPASS_OBJECT) return DRAWRESULT_SKIP;

	BaseContainer *bc=op->GetDataInstance();

	//Real bounce=-bc->GetReal(HAIRSDK_COLLIDER_BOUNCE);
	//Real friction=bc->GetReal(HAIRSDK_COLLIDER_FRICTION);
	Real width=bc->GetReal(HAIRSDK_COLLIDER_WIDTH);
	Real height=bc->GetReal(HAIRSDK_COLLIDER_HEIGHT);

	const Matrix &mg = bh->GetMg();

	bd->SetPen(bd->GetObjectColor(bh,op));
	bd->SetMatrix_Matrix(op, mg);

	bd->DrawLine(Vector(-width,height,0.0),Vector(width,height,0.0), 0);
	bd->DrawLine(Vector(width,height,0.0),Vector(width,-height,0.0), 0);
	bd->DrawLine(Vector(width,-height,0.0),Vector(-width,-height,0.0), 0);
	bd->DrawLine(Vector(-width,-height,0.0),Vector(-width,height,0.0), 0);

	return DRAWRESULT_OK;
}

//////////////////////////////////////////////////////////////////////////

#define ID_HAIR_COLLIDER_EXAMPLE 1018963

Bool RegisterCollisionObject()
{
	return RegisterObjectPlugin(ID_HAIR_COLLIDER_EXAMPLE,GeLoadString(IDS_HAIR_COLLIDER_EXAMPLE),OBJECT_PARTICLEMODIFIER,HairCollisionObject::Alloc,"Ohairsdkcollider",AutoBitmap("haircollider.tif"),0);
}
