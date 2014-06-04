/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2007 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code for creating a tag that can use the Hair API

//////////////////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"
#include "lib_hair.h"

#include "thairsdkrendering.h"

//////////////////////////////////////////////////////////////////////////

class HairRenderingTag : public TagData
{
	INSTANCEOF(HairRenderingTag,TagData)

public:

	virtual Bool Init(GeListNode *node);
	virtual void Free(GeListNode *node);

	virtual Bool Message(GeListNode *node, LONG type, void *data);

	static NodeData *Alloc(void) { return gNew HairRenderingTag; }

	//////////////////////////////////////////////////////////////////////////
	
	HairPluginObjectData m_FnTable;

	Real m_Shadow;
	Real m_Trans;
	LONG m_Depth;
};

//////////////////////////////////////////////////////////////////////////

Bool _InitRenderFn(HairVideoPost *vp, VolumeData *vd, BaseDocument *doc, BaseList2D *bl, HairObject *op, HairGuides *guides, LONG oindex, LONG pass)
{
	//GePrint("Init Render");

	BaseContainer *bc=bl->GetDataInstance();
	HairRenderingTag *hrt=(HairRenderingTag*)bl->GetNodeData();

	if (!bc || !hrt) return FALSE;

	hrt->m_Shadow=bc->GetReal(HAIR_RENDERING_SHADOW);
	hrt->m_Trans=bc->GetReal(HAIR_RENDERING_TRANSPARENCY);
	hrt->m_Depth=bc->GetLong(HAIR_RENDERING_DEPTH);

	return TRUE;
}

void _HrFreeRenderFn(HairVideoPost *vp, BaseList2D *bl)
{
	//GePrint("Free Render");
}

Real _ModifyHairShadowTransparencyFn(HairVideoPost *vp, LONG oindex, HairMaterialData *mat, RayObject *ro, HairObject *op, HairGuides *guides, BaseList2D *bl, Real *thk, VolumeData *vd, LONG cpu, LONG lid, LONG seg, LONG p, Real lined, const Vector &linep, const Vector &n, const Vector &lp, const Vector &huv, const RayHitID &ply_id, RayLight *light, Real trans)
{
	HairRenderingTag *hrt=(HairRenderingTag*)bl->GetNodeData();

	if (light)	// shadow call
		trans=RCO 1.0-FCut01((RCO 1.0-FCut01(trans))*hrt->m_Shadow);
	else
		trans=RCO 1.0-FCut01((RCO 1.0-FCut01(trans))*hrt->m_Trans);

	if (hrt->m_Depth>1)
	{
		Real depth=Real(1+(seg%hrt->m_Depth))/Real(hrt->m_Depth);
		trans=1.0-((1.0-trans)*depth);
	}

	return trans;
}

Vector _GenerateColorFn(HairVideoPost *vp, LONG oindex, HairMaterialData *mat, RayObject *ro, HairObject *op, HairGuides *guides, BaseList2D *bl, Real *thk, VolumeData *vd, LONG cpu, LONG lid, LONG seg, LONG p, Real lined, const Vector &linep, const Vector &v, const Vector &n, const Vector &lp, const Vector &t, const Vector &r, const Vector &huv, const RayHitID &ply_id)
{
	HairRenderingTag *hrt=(HairRenderingTag*)bl->GetNodeData();

	Vector col;

	vp->Sample(oindex,vd,cpu,lid,seg,p,lined,linep,v,col,n,lp,t,r,huv,HAIR_VP_FLAG_NOHOOKS);

	if (hrt->m_Depth>1)
	{
		Real depth=Real(1+(seg%hrt->m_Depth))/Real(hrt->m_Depth);
		col=col*depth;
	}

	return col;
}

//////////////////////////////////////////////////////////////////////////

Bool HairRenderingTag::Init(GeListNode *node)
{
	BaseContainer *bc=((BaseList2D*)node)->GetDataInstance();
	
	m_FnTable.init_render=_InitRenderFn;
	m_FnTable.free_render=_HrFreeRenderFn;
	m_FnTable.calc_shad=_ModifyHairShadowTransparencyFn;
	m_FnTable.calc_col=_GenerateColorFn;

	bc->SetReal(HAIR_RENDERING_SHADOW,1.0);
	bc->SetReal(HAIR_RENDERING_TRANSPARENCY,1.0);
	bc->SetLong(HAIR_RENDERING_DEPTH,1);

	return TRUE;
}

void HairRenderingTag::Free(GeListNode *node)
{
}

Bool HairRenderingTag::Message(GeListNode *node, LONG type, void *data)
{
	if (type==MSG_HAIR_GET_OBJECT_TYPE && data)
	{
		HairPluginMessageData *mdata=(HairPluginMessageData*)data;
		mdata->data=&m_FnTable;
		return TRUE;
	}
	
	return SUPER::Message(node,type,data);
}

//////////////////////////////////////////////////////////////////////////

#define ID_HAIR_RENDERING_EXAMPLE 1018984

Bool RegisterRenderingTag()
{
	return RegisterTagPlugin(ID_HAIR_RENDERING_EXAMPLE,GeLoadString(IDS_HAIR_RENDERING_EXAMPLE),TAG_MULTIPLE|TAG_VISIBLE,HairRenderingTag::Alloc,"Thairsdkrendering",AutoBitmap("hairrendering.tif"),0);
}
