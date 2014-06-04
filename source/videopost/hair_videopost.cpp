/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2007 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code of interacting with Hair at render time

//////////////////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"

#include "lib_hair.h"

class HairSDKVideopost : public VideoPostData
{
	INSTANCEOF(HairSDKVideopost,VideoPostData)

public:

	virtual Bool Init(GeListNode *node);
	static NodeData *Alloc(void) { return gNew HairSDKVideopost; }

	virtual RENDERRESULT Execute(BaseVideoPost *node, VideoPostStruct *vps);
	virtual VIDEOPOSTINFO GetRenderInfo(BaseVideoPost *node) { return VIDEOPOSTINFO_0; }

	virtual Bool RenderEngineCheck(BaseVideoPost *node, LONG id);

	void *m_pOldColorHook;
}; 

//////////////////////////////////////////////////////////////////////////

Vector _SampleHairColorHook(HairVideoPost *vp, LONG oindex, HairMaterialData *mat, RayObject *ro, HairObject *op, HairGuides *guides, BaseList2D *bl, Real *thk, VolumeData *vd, LONG cpu, LONG lid, LONG seg, LONG p, Real lined, const Vector &linep, const Vector &v, const Vector &n, const Vector &lp, const Vector &t, const Vector &r, const Vector &huv, LONG ply_id)
{
	return Vector(1,0,0);
}

Real _SampleHairTransparencyHook(HairVideoPost *vp, LONG oindex, HairMaterialData *mat, RayObject *ro, HairObject *op, HairGuides *guides, BaseList2D *bl, Real *thk, VolumeData *vd, LONG cpu, LONG lid, LONG seg, LONG p, Real lined, const Vector &linep, const Vector &n, const Vector &lp, const Vector &huv, LONG ply_id)
{
	return 1.0;
}

Real _SampleShadowBufferHook(HairVideoPost *vp, VolumeData *vd, RayLight *light, const Vector &p, Real delta, LONG cpu)
{
	return 1.0;
}

Bool _IlluminateHook(HairVideoPost *vp, VolumeData *vd, RayLight *light, Vector &colshad, Vector &col, Vector &lv, const Vector &p, const Vector &v)
{
	colshad=col=Vector(1);
	lv=-!(p-(vd->GetRayCamera()->m.off).ToRV());
	return TRUE;
}

Bool HairSDKVideopost::Init(GeListNode *node)
{
	//BaseVideoPost *pp = (BaseVideoPost*)node;
	//BaseContainer  *dat = pp->GetDataInstance();

	return TRUE;
}

Bool HairSDKVideopost::RenderEngineCheck(BaseVideoPost *node, LONG id)
{
	// the following render engines are not supported by this effect
	if (id==RDATA_RENDERENGINE_PREVIEWSOFTWARE || 
			id==RDATA_RENDERENGINE_PREVIEWHARDWARE || 
			id==RDATA_RENDERENGINE_CINEMAN) 
		return FALSE;

	return TRUE; 
}

RENDERRESULT HairSDKVideopost::Execute(BaseVideoPost *node, VideoPostStruct *vps)
{
	if (vps->vp==VIDEOPOSTCALL_RENDER)
	{
		HairLibrary hlib;

		if (vps->open)
		{
			m_pOldColorHook=hlib.SetHook(vps->doc,HAIR_HOOK_TYPE_SAMPLE_COLOR,(void*)_SampleHairColorHook);
			//hlib.SetHook(vps->doc,HAIR_HOOK_TYPE_SAMPLE_TRANS,_SampleHairTransparencyHook);
			//hlib.SetHook(vps->doc,HAIR_HOOK_TYPE_SAMPLE_SHADOWS,_SampleShadowBufferHook);
			//hlib.SetHook(vps->doc,HAIR_HOOK_TYPE_ILLUMINATE,_IlluminateHook);
		}
		else
		{
			hlib.SetHook(vps->doc,HAIR_HOOK_TYPE_SAMPLE_COLOR,m_pOldColorHook);
		}
	}

	return RENDERRESULT_OK;	
}


#define ID_HAIR_COLLIDER_EXAMPLE 1018971

Bool RegisterVideopost()
{
	return RegisterVideoPostPlugin(ID_HAIR_COLLIDER_EXAMPLE,GeLoadString(IDS_HAIR_VIDEOPOST_EXAMPLE),PLUGINFLAG_VIDEOPOST_MULTIPLE,HairSDKVideopost::Alloc,"VPhairsdkpost",0,0);
}
