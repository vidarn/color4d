/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// video post example file - colorize
// this video post effect shows
// - storage of posteffects settings
// - allocation of buffers
// - parameter description of VP effect
// - use of executeline

#include "c4d.h"
#include "c4d_symbols.h"
#include "vpcolorize.h"
#include "customgui_lensglow.h"

class ColorizeData : public VideoPostData
{
	INSTANCEOF(ColorizeData,VideoPostData)

	private:
		VPBuffer	*buf;
		Real			dr,dg,db;

	public:
		virtual Bool Init(GeListNode *node);
		static NodeData *Alloc(void) { return gNew ColorizeData; }

		virtual void AllocateBuffers(BaseVideoPost *node, Render *render, BaseDocument *doc);
		virtual RENDERRESULT Execute(BaseVideoPost *node, VideoPostStruct *vps);
		virtual void ExecuteLine(BaseVideoPost *node, PixelPost *pp);
		virtual VIDEOPOSTINFO GetRenderInfo(BaseVideoPost *node) { return VIDEOPOSTINFO_EXECUTELINE; }
		virtual Bool GetDEnabling(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc);
		virtual Bool RenderEngineCheck(BaseVideoPost *node, LONG id);
}; 

Bool ColorizeData::Init(GeListNode *node)
{
	BaseVideoPost *pp = (BaseVideoPost*)node;
	BaseContainer  *dat = pp->GetDataInstance();

	dat->SetReal(VP_COLORIZE_DELTA_R, 0.2);
	dat->SetReal(VP_COLORIZE_DELTA_G, 0.0);
	dat->SetReal(VP_COLORIZE_DELTA_B,-0.2);
	dat->SetLong(VP_COLORIZE_MODE,VP_COLORIZE_MODE_COMPLETE);
	dat->SetLong(VP_COLORIZE_OBJECTID,1);
	dat->SetData(VP_COLORIZE_LENSGLOW,GeData(CUSTOMDATATYPE_LENSGLOW,DEFAULTVALUE));

	return TRUE;
}

Bool ColorizeData::GetDEnabling(GeListNode *node, const DescID &id,const GeData &t_data,DESCFLAGS_ENABLE flags,const BaseContainer *itemdesc)
{
	BaseContainer *data = ((BaseObject*)node)->GetDataInstance();
	switch (id[0].id)
	{
		case VP_COLORIZE_OBJECTID: return data->GetLong(VP_COLORIZE_MODE)==VP_COLORIZE_MODE_OBJECTBUFFER;
	}
	return TRUE;
}

RENDERRESULT ColorizeData::Execute(BaseVideoPost *node, VideoPostStruct *vps)
{
	if (vps->vp==VIDEOPOSTCALL_FRAMESEQUENCE && vps->open)
	{
		BaseContainer *dat = node->GetDataInstance();
		LONG obj	= dat->GetLong(VP_COLORIZE_OBJECTID,1);
		LONG mode = dat->GetLong(VP_COLORIZE_MODE);
	
		// value caching for faster access
		dr  = dat->GetReal(VP_COLORIZE_DELTA_R)+1.0;
		dg  = dat->GetReal(VP_COLORIZE_DELTA_G)+1.0;
		db  = dat->GetReal(VP_COLORIZE_DELTA_B)+1.0;
		buf = NULL;

		switch (mode)
		{
			case 1: buf = vps->render->GetBuffer(VPBUFFER_ALPHA, 0); break;
			case 2: buf = vps->render->GetBuffer(VPBUFFER_OBJECTBUFFER,obj); break;
		}
	}
	else if (vps->vp==VIDEOPOSTCALL_INNER && !vps->open)
	{
		BaseContainer *dat = node->GetDataInstance();
		GeData gd = dat->GetData(VP_COLORIZE_LENSGLOW);
		LensGlowStruct*	cd = (LensGlowStruct*)gd.GetCustomDataType(CUSTOMDATATYPE_LENSGLOW);
		if (cd)
		{
			Vector pos=0.5;
			vps->vd->AddLensGlow(cd,&pos,1,1.0,FALSE);
			
			VPBuffer *rgba=vps->render->GetBuffer(VPBUFFER_RGBA,NOTOK);
			if (rgba)
				vps->vd->SampleLensFX(rgba,NULL,vps->thread);
		}
	}
		
	return RENDERRESULT_OK;	
}

void ColorizeData::AllocateBuffers(BaseVideoPost *node, Render *render, BaseDocument *doc)
{
	BaseContainer *dat = node->GetDataInstance();
	LONG mode = dat->GetLong(VP_COLORIZE_MODE);

	LONG needbitdepth = 8; // also 16 and 32 are possible

	switch (mode)
	{
		case VP_COLORIZE_MODE_ALPHA				: render->AllocateBuffer(VPBUFFER_ALPHA, 0, needbitdepth, FALSE); break;
		case VP_COLORIZE_MODE_OBJECTBUFFER: render->AllocateBuffer(VPBUFFER_OBJECTBUFFER, dat->GetLong(VP_COLORIZE_OBJECTID,1), needbitdepth, FALSE); break;
	}
}

void ColorizeData::ExecuteLine(BaseVideoPost *node, PixelPost *pp)
{
	LONG alphacomp = NOTOK;
	LONG mul   = pp->aa ? 4:1;
	SReal *col  = pp->col;
	Real grey,aa,na;
	LONG i,x;

	if (buf) alphacomp = buf->GetInfo(VPGETINFO_LINEOFFSET);

	if (alphacomp!=NOTOK)
	{
		for (x=pp->xmin; x<=pp->xmax; x++)
		{
			for (i=0; i<mul; i++, col+=pp->comp)
			{
				grey   = (col[0]+col[1]+col[2])/3.0;
				aa     = col[alphacomp];
				na     = 1.0-aa;
				col[0] = aa * grey*dr + na*col[0];
				col[1] = aa * grey*dg + na*col[1];
				col[2] = aa * grey*db + na*col[2];
			}
		}
	}
	else
	{
		for (x=pp->xmin; x<=pp->xmax; x++)
		{
			for (i=0; i<mul; i++, col+=pp->comp)
			{
				grey   = (col[0]+col[1]+col[2])/3.0;
				col[0] = grey*dr;
				col[1] = grey*dg;
				col[2] = grey*db;
			}
		}
	}
}

Bool ColorizeData::RenderEngineCheck(BaseVideoPost *node, LONG id)
{
	// the following render engines are not supported by this effect
	if (id==RDATA_RENDERENGINE_PREVIEWSOFTWARE || 
			id==RDATA_RENDERENGINE_PREVIEWHARDWARE || 
			id==RDATA_RENDERENGINE_CINEMAN) 
		return FALSE;

	return TRUE; 
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_COLORIZEVIDEOPOST 1000968

Bool RegisterVPTest(void)
{
	return RegisterVideoPostPlugin(ID_COLORIZEVIDEOPOST,GeLoadString(IDS_VIDEOPOST),PLUGINFLAG_VIDEOPOST_MULTIPLE,ColorizeData::Alloc,"VPcolorize",0,0);
}
