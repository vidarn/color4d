/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2007 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code for creating a dynamics force object for Hair

//////////////////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_hair.h"

#include "ohairsdkforce.h"

//////////////////////////////////////////////////////////////////////////

class HairForceObject : public ObjectData
{
	INSTANCEOF(HairForceObject,ObjectData)

public:

	virtual Bool Init(GeListNode *node);
	virtual void Free(GeListNode *node);

	virtual Bool Message(GeListNode *node, LONG type, void *data);
	virtual DRAWRESULT Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh);

	static NodeData *Alloc(void) { return gNew HairForceObject; }

	//////////////////////////////////////////////////////////////////////////
	
	HairPluginObjectData m_FnTable;
};

//////////////////////////////////////////////////////////////////////////

static Bool _ForceFn(BaseDocument *doc, BaseList2D *op, HairObject *hair, HairGuides *guides, HairGuideDynamics *dyn, Vector *force, Real *invmass, Vector *padr, LONG pcnt, LONG cnt, LONG scnt, Real t1, Real t2)
{
	BaseContainer *bc=op->GetDataInstance();

	BaseObject *pObject=(BaseObject*)bc->GetLink(HAIR_FORCE_LINK,doc,Obase);
	if (!pObject) return TRUE;

	Real strength=bc->GetReal(HAIR_FORCE_STRENGTH);

	LONG i,l,j;
	Matrix mg=pObject->GetMg();

	for (i=0;i<cnt;i++)
	{
		for (l=1;l<scnt;l++)
		{
			j=i*scnt+l;

			Vector f=padr[j]-mg.off;
			Real d=Len(f);

			if (d<1e-5) continue;

			f=strength*f/d;
			force[j]-=f;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

Bool HairForceObject::Init(GeListNode *node)
{
	BaseContainer *bc=((BaseList2D*)node)->GetDataInstance();
	
	bc->SetReal(HAIR_FORCE_STRENGTH,1.0);
	
	m_FnTable.calc_force=_ForceFn;

	return TRUE;
}

void HairForceObject::Free(GeListNode *node)
{
}

Bool HairForceObject::Message(GeListNode *node, LONG type, void *data)
{
	if (type==MSG_HAIR_GET_OBJECT_TYPE && data)
	{
		HairPluginMessageData *mdata=(HairPluginMessageData*)data;
		mdata->data=&m_FnTable;
		return TRUE;
	}
	
	return SUPER::Message(node,type,data);
}

DRAWRESULT HairForceObject::Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh)
{
	return DRAWRESULT_SKIP;
}

//////////////////////////////////////////////////////////////////////////

#define ID_HAIR_FORCE_EXAMPLE 1018962

Bool RegisterForceObject()
{
	return RegisterObjectPlugin(ID_HAIR_FORCE_EXAMPLE,GeLoadString(IDS_HAIR_FORCE_EXAMPLE),OBJECT_PARTICLEMODIFIER,HairForceObject::Alloc,"Ohairsdkforce",AutoBitmap("hairforce.tif"),0);
}
