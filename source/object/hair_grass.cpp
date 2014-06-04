/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2007 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code for creating a Hair render time generator object

//////////////////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_hair.h"
#include "ohairsdkgrass.h"

//////////////////////////////////////////////////////////////////////////

#define HAIRSTYLE_LINK	1000
#define HAIRSTYLE_FUR_COUNT 1104

//////////////////////////////////////////////////////////////////////////

class HairGrassObject : public ObjectData
{
	INSTANCEOF(HairGrassObject,ObjectData)

public:

	virtual Bool Init(GeListNode *node);
	virtual void Free(GeListNode *node);

	virtual Bool Message(GeListNode *node, LONG type, void *data);
	virtual DRAWRESULT Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh);

	static NodeData *Alloc(void) { return gNew HairGrassObject; }

	//////////////////////////////////////////////////////////////////////////
	
	HairPluginObjectData m_FnTable;

	LONG m_Count,m_Segments;
	PolygonObject *m_pPoly;
	Real m_Length,m_LengthVar,m_Noise;
	Random m_Rnd;
};

//////////////////////////////////////////////////////////////////////////

Vector _GenerateColor(HairVideoPost *vp, LONG oindex, HairMaterialData *mat, RayObject *ro, HairObject *op, HairGuides *guides, BaseList2D *bl, Real *thk, VolumeData *vd, LONG cpu, LONG lid, LONG seg, LONG p, Real lined, const Vector &linep, const Vector &v, const Vector &n, const Vector &lp, const Vector &t, const Vector &r, const Vector &huv, const RayHitID &ply_id)
{
	LONG scnt=guides->GetSegmentCount();

	Real p1=Real(p-1)/Real(scnt);
	Real p2=Real(p)/Real(scnt);

	Real dlt=1.0-Mix(p1,p2,lined);
	Real tdlt=Mix(thk[p-1],thk[p],lined);

	LONG l;
	Real shd=1.0;

	for (l=0;l<vd->GetLightCount();l++)
	{
		shd*=vp->SampleShadow(vd,vd->GetLight(l),linep,tdlt,cpu,0);
	}

	return Vector(0,0.5*dlt,0)*shd;
}

Real _GenerateTransparency(HairVideoPost *vp, LONG oindex, HairMaterialData *mat, RayObject *ro, HairObject *op, HairGuides *guides, BaseList2D *bl, Real *thk, VolumeData *vd, LONG cpu, LONG lid, LONG seg, LONG p, Real lined, const Vector &linep, const Vector &n, const Vector &lp, const Vector &huv, const RayHitID &ply_id, RayLight *light)
{
	LONG scnt=guides->GetSegmentCount();

	Real p1=Real(p-1)/Real(scnt-1);
	Real p2=Real(p)/Real(scnt-1);

	Real dlt=1.0-Mix(p1,p2,lined);

	return 1.0-(0.8*dlt);
}

HairGuides *_GenerateFn(BaseDocument *doc, BaseList2D *op, HairObject *hair, BaseThread *thd, VolumeData *vd, LONG pass, void *data)
{
	HairGrassObject *grass=(HairGrassObject*)op->GetNodeData();
	BaseContainer *bc=op->GetDataInstance();
	HairGuides *hairs=NULL;

	switch (pass)
	{
	case HAIR_GENERATE_PASS_INIT:
	{
		grass->m_pPoly=(PolygonObject*)bc->GetLink(HAIR_GRASS_LINK,doc,Opolygon);
		if (!grass->m_pPoly) return NULL;

		grass->m_Count=bc->GetLong(HAIR_GRASS_COUNT);
		grass->m_Segments=bc->GetLong(HAIR_GRASS_SEGMENTS);
		grass->m_Length=bc->GetReal(HAIR_GRASS_LENGTH);
		grass->m_LengthVar=bc->GetReal(HAIR_GRASS_LENGTH);
		grass->m_Noise=bc->GetReal(HAIR_GRASS_NOISE);
		
		BaseContainer *hbc=hair->GetDataInstance();

		hbc->SetLink(HAIRSTYLE_LINK,grass->m_pPoly);
		hbc->SetLong(HAIRSTYLE_FUR_COUNT,grass->m_Count);

		break;
	}
	case HAIR_GENERATE_PASS_BUILD:
		if (bc->GetBool(HAIR_GRASS_GENERATE))
		{
			hairs=HairGuides::Alloc(grass->m_Count,grass->m_Segments);
			if (hairs) hair->SetGuides(hairs,FALSE);
			hairs=hair->GetGuides();

			if (hairs)
			{
				Vector *pnts=hairs->GetPoints(),n(DC),r(DC);
				LONG i,scnt=(grass->m_Segments+1),l=0,ply;
				//const Vector *padr=grass->m_pPoly->GetPointR();
				const CPolygon *vadr=grass->m_pPoly->GetPolygonR();
				LONG vcnt=grass->m_pPoly->GetPolygonCount();
				Real s,t;
				HairRootData hroot;
					
				hroot.m_Type=HAIR_ROOT_TYPE_POLY;

				grass->m_Rnd.Init(4729848);

				for (i=0;i<grass->m_Count;i++)
				{
					ply=LONG(grass->m_Rnd.Get01()*Real(vcnt-1));

					if (vadr[ply].c==vadr[ply].d)
					{
						do { s=grass->m_Rnd.Get01(),t=grass->m_Rnd.Get01(); }
						while ((s+t)>1.0);
					}
					else
					{
						s=grass->m_Rnd.Get01(),t=grass->m_Rnd.Get01();
					}

					hroot.m_ID=ply;
					hroot.m_S=s;
					hroot.m_T=t;

					hairs->SetRoot(0,hroot,FALSE);
					hairs->GetRootData(0,&r,&n,NULL,FALSE);
						
					Real len=grass->m_Length+grass->m_LengthVar*grass->m_Rnd.Get11();
					Vector dn=n;
					Matrix axis;

					hairs->GetRootAxis(0,axis,FALSE,FALSE);

					dn.x+=grass->m_Noise*SNoise(Vector(ply,s,t));
					dn.y+=grass->m_Noise*SNoise(Vector(i,ply,t));
					dn.z+=grass->m_Noise*SNoise(Vector(ply,s,i));

					dn=(!dn)*len;

					for (l=0;l<scnt;l++)
					{
						pnts[i*scnt+l]=r+dn*Real(l)/Real(scnt-1);
					}
				}
			}
		}
		else
			hairs=HairGuides::Alloc(1,grass->m_Segments);

		break;
	case HAIR_GENERATE_PASS_FREE:
		break;
	}

	return hairs;
}

LONG _CalcHair(LONG index, LONG oindex, NodeData *node, HairGuides *guides, Vector *guide_pnts, Vector *rend_pnts, Real *thickness, VolumeData *vd, Vector *n)
{
	HairGrassObject *grass=(HairGrassObject*)node;
	Vector r(DC);
	LONG scnt=(grass->m_Segments+1),l=0,ply;
	//const Vector *padr=grass->m_pPoly->GetPointR();
	const CPolygon *vadr=grass->m_pPoly->GetPolygonR();
	LONG vcnt=grass->m_pPoly->GetPolygonCount();
	Real s,t;
	HairRootData hroot;

	if (index==0) grass->m_Rnd.Init(4729848);
		
	hroot.m_Type=HAIR_ROOT_TYPE_POLY;

	ply=LONG(grass->m_Rnd.Get01()*Real(vcnt-1));

	if (vadr[ply].c==vadr[ply].d)
	{
		do { s=grass->m_Rnd.Get01(),t=grass->m_Rnd.Get01(); }
		while ((s+t)>1.0);
	}
	else
	{
		s=grass->m_Rnd.Get01(),t=grass->m_Rnd.Get01();
	}

	hroot.m_ID=ply;
	hroot.m_S=s;
	hroot.m_T=t;

	guides->SetRoot(0,hroot,FALSE);
	guides->GetRootData(0,&r,n,NULL,FALSE);
		
	Real len=grass->m_Length+grass->m_LengthVar*grass->m_Rnd.Get11();
	if (len<=0.0) return HAIR_CALC_FLAG_SKIP;

	Vector dn=*n;

	Matrix axis;

	guides->GetRootAxis(0,axis,FALSE,FALSE);

	dn.x+=grass->m_Noise*SNoise(Vector(ply,s,t));
	dn.y+=grass->m_Noise*SNoise(Vector(index,ply,t));
	dn.z+=grass->m_Noise*SNoise(Vector(ply,s,index));

	dn=(!dn)*len;

	for (l=0;l<scnt;l++)
	{
		guide_pnts[l]=rend_pnts[l]=r+dn*Real(l)/Real(scnt-1);
	}

	return HAIR_CALC_FLAG_APPLYMATERIALS;
}

//////////////////////////////////////////////////////////////////////////

Bool HairGrassObject::Init(GeListNode *node)
{
	BaseContainer *bc=((BaseList2D*)node)->GetDataInstance();
	
	bc->SetLong(HAIR_GRASS_COUNT,5000);
	bc->SetLong(HAIR_GRASS_SEGMENTS,6);
	bc->SetReal(HAIR_GRASS_LENGTH,15);
	bc->SetReal(HAIR_GRASS_LENGTH_VAR,5);
	bc->SetReal(HAIR_GRASS_NOISE,0.2);
	bc->SetBool(HAIR_GRASS_GENERATE,FALSE);

	return TRUE;
}

void HairGrassObject::Free(GeListNode *node)
{
}

Bool HairGrassObject::Message(GeListNode *node, LONG type, void *data)
{
	if (type==MSG_HAIR_GET_OBJECT_TYPE && data)
	{
		HairPluginMessageData *mdata=(HairPluginMessageData*)data;
		BaseContainer *bc=((BaseList2D*)node)->GetDataInstance();

		m_FnTable.calc_generate=_GenerateFn;
		m_FnTable.calc_col=_GenerateColor;
		m_FnTable.calc_trans=_GenerateTransparency;

		if (!bc->GetBool(HAIR_GRASS_GENERATE))
			m_FnTable.calc_hair=_CalcHair;
		else
			m_FnTable.calc_hair=NULL;

		mdata->data=&m_FnTable;

		return TRUE;
	}
	
	return SUPER::Message(node,type,data);
}

DRAWRESULT HairGrassObject::Draw(BaseObject* op, DRAWPASS drawpass, BaseDraw* bd, BaseDrawHelp* bh)
{
	return DRAWRESULT_SKIP;
}

//////////////////////////////////////////////////////////////////////////

#define ID_HAIR_GRASS_EXAMPLE 1018965

Bool RegisterGrassObject()
{
	return RegisterObjectPlugin(ID_HAIR_GRASS_EXAMPLE,GeLoadString(IDS_HAIR_GRASS_EXAMPLE),OBJECT_GENERATOR,HairGrassObject::Alloc,"Ohairsdkgrass",AutoBitmap("hairgrass.tif"),0);
}
