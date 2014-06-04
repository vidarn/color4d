/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example for a channel shader with access to basechannel
// using standard GUI elements

#include "c4d.h"
#include "c4d_symbols.h"
#include "xbitmapdistortion.h"

class BitmapData : public ShaderData
{
	public:
		Real					noise,scale,octaves;
		BaseShader	*shader;
	public:
		virtual Bool Init		(GeListNode *node);
		virtual Bool Message(GeListNode *node, LONG type, void *data);
		virtual	Vector Output		(BaseShader *chn, ChannelData *cd);
		virtual Bool Read(GeListNode *node, HyperFile *hf, LONG level);

		virtual	INITRENDERRESULT InitRender	(BaseShader *chn, const InitRenderStruct &irs);
		virtual	void FreeRender	(BaseShader *chn);

		virtual	SHADERINFO GetRenderInfo(BaseShader *sh);

		static NodeData *Alloc(void) { return gNew BitmapData; }
};

SHADERINFO BitmapData::GetRenderInfo(BaseShader *sh)
{
	return SHADERINFO_BUMP_SUPPORT;
}

Bool BitmapData::Init(GeListNode *node)
{
	shader=NULL;

	BaseContainer *data = ((BaseShader*)node)->GetDataInstance();
	
	data->SetReal(BITMAPDISTORTIONSHADER_NOISE,0.0);
	data->SetReal(BITMAPDISTORTIONSHADER_OCTAVES,1.0);
	data->SetReal(BITMAPDISTORTIONSHADER_SCALE,1.0);
	data->SetLink(BITMAPDISTORTIONSHADER_TEXTURE,NULL);

	return TRUE;
}

Bool BitmapData::Read(GeListNode *node, HyperFile *hf, LONG level)
{
	if (hf->GetFileVersion()<8300)
	{
		if (!hf->ReadChannelConvert(node,BITMAPDISTORTIONSHADER_TEXTURE)) return FALSE; // convert old basechannel
	}

	return TRUE;
}

Vector BitmapData::Output(BaseShader *chn, ChannelData *cd)
{
	if (!shader) return 1.0;

	Vector uv=cd->p;

	if (noise>0.0)
	{
		Real    scl = 5.0*scale;
		Vector  res = Vector(Turbulence(uv*scl,octaves,TRUE),Turbulence((uv+Vector(0.34,13.0,2.43))*scl,octaves,TRUE),0.0);
		cd->p.x  = Mix(uv.x,res.x,noise);
		cd->p.y  = Mix(uv.y,res.y,noise);
	}

	Vector res=shader->Sample(cd);
	cd->p=uv;

	return res;
}

INITRENDERRESULT BitmapData::InitRender(BaseShader *chn, const InitRenderStruct &irs)
{
	BaseContainer *data = chn->GetDataInstance();

  // cache values for fast access
	noise   = data->GetReal(BITMAPDISTORTIONSHADER_NOISE);
	octaves = data->GetReal(BITMAPDISTORTIONSHADER_OCTAVES);
	scale   = data->GetReal(BITMAPDISTORTIONSHADER_SCALE);
	shader  = (BaseShader*)data->GetLink(BITMAPDISTORTIONSHADER_TEXTURE,irs.doc,Xbase);
	if (shader) 
		return shader->InitRender(irs);

	return INITRENDERRESULT_OK; 
}

void BitmapData::FreeRender(BaseShader *chn)
{
	if (shader)
	  shader->FreeRender();
	shader=NULL;
}

Bool BitmapData::Message(GeListNode *node, LONG type, void *msgdat)
{
	BaseContainer *data = ((BaseShader*)node)->GetDataInstance();

	HandleInitialChannel(node,BITMAPDISTORTIONSHADER_TEXTURE,type,msgdat);
	HandleShaderMessage(node,(BaseShader*)data->GetLink(BITMAPDISTORTIONSHADER_TEXTURE,node->GetDocument(),Xbase),type,msgdat);

	return TRUE;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_BITMAPDISTORTION 1001160

Bool RegisterBitmap(void)
{
	return RegisterShaderPlugin(ID_BITMAPDISTORTION,GeLoadString(IDS_BITMAPDISTORTION),0,BitmapData::Alloc,"Xbitmapdistortion",0);
}

