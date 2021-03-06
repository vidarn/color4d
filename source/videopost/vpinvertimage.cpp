/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2009 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// video post example file - invert image
// simple video post effect 
// - usage of VPBuffer
// - operates after the image is completely rendered

#include "c4d.h"
#include "c4d_symbols.h"
#include "c4d_gl.h"

class InvertData : public VideoPostData
{
	public:
		static NodeData *Alloc(void) { return gNew InvertData; }
		virtual RENDERRESULT Execute(BaseVideoPost *node, VideoPostStruct *vps);
		virtual void Free(GeListNode* node);
		virtual VIDEOPOSTINFO GetRenderInfo(BaseVideoPost *node) { return VIDEOPOSTINFO_0; }
		virtual Bool GlDraw(BaseVideoPost *node, BaseDraw *bd, GlFrameBuffer* fbuf, LONG colortex, LONG depthtex, VIDEOPOST_GLDRAW flags);
		virtual VIDEOPOST_GLINFO GetGlInfo(BaseVideoPost *node, BaseDocument* doc, BaseDraw *bd);
		virtual Bool RenderEngineCheck(BaseVideoPost *node, LONG id);

	protected:
		static void* AllocCgDescription();
		static void FreeCgDescription(void* pData);
		static Bool ReadCgDescription(GlReadDescriptionData* pFile, void* pData);
		static Bool WriteCgDescription(GlWriteDescriptionData* pFile, const void* pData);
};

struct VPInvertDescData 
{
	GlString strTexsize, strTexture;
	GlProgramParameter paramTexsize, paramTexture;
	LONG lVectorCount;
	const GlVertexBufferVectorInfo* const* ppVectorInfo;
};

void* InvertData::AllocCgDescription()
{
	return gNew VPInvertDescData;
}

void InvertData::FreeCgDescription(void* pData)
{
	VPInvertDescData* pDescData = (VPInvertDescData*)pData;
	gDelete(pDescData);
}

Bool InvertData::ReadCgDescription(GlReadDescriptionData* pFile, void* pData)
{
	VPInvertDescData* pDesc = (VPInvertDescData*)pData;
	if (!GlProgramFactory::ReadParameter(pFile, pDesc->paramTexsize)) return FALSE;
	if (!GlProgramFactory::ReadParameter(pFile, pDesc->paramTexture)) return FALSE;
	return TRUE;
}

Bool InvertData::WriteCgDescription(GlWriteDescriptionData* pFile, const void* pData)
{
	const VPInvertDescData* pDesc = (const VPInvertDescData*)pData;
	if (!GlProgramFactory::WriteParameter(pFile, pDesc->paramTexsize)) return FALSE;
	if (!GlProgramFactory::WriteParameter(pFile, pDesc->paramTexture)) return FALSE;
	return TRUE;
}

RENDERRESULT InvertData::Execute(BaseVideoPost *node, VideoPostStruct *vps)
{
	if (vps->vp==VIDEOPOSTCALL_RENDER && !vps->open && *vps->error==RENDERRESULT_OK && !vps->thread->TestBreak())
	{
		VPBuffer *rgba    = vps->render->GetBuffer(VPBUFFER_RGBA,NOTOK);
		RayParameter *ray = vps->vd->GetRayParameter(); // only in VP_INNER & VIDEOPOSTCALL_RENDER
		if (!ray)  return RENDERRESULT_OUTOFMEMORY;
		if (!rgba) return RENDERRESULT_OUTOFMEMORY;

		LONG x1,y1,x2,y2,x,y,cnt,i;

		// example functions
		LONG cpp   = rgba->GetInfo(VPGETINFO_CPP);

		x1 = ray->left;
		y1 = ray->top;
		x2 = ray->right;
		y2 = ray->bottom;
		cnt = x2-x1+1;

		SReal *b,*buffer = GeAllocType(SReal,cpp*cnt);
		if (!buffer) return RENDERRESULT_OUTOFMEMORY;

		for (y=y1;y<=y2;y++)
		{
			rgba->GetLine(x1,y,cnt,buffer,32,TRUE);

			for (b=buffer,x=x1;x<=x2;x++,b+=cpp)
			{
				for (i=0;i<3;i++)
					b[i] = 1.0-b[i];
			}

			rgba->SetLine(x1,y,cnt,buffer,32,TRUE);
		}
		GeFree(buffer);
	}
		
	return RENDERRESULT_OK;	
}

void InvertData::Free(GeListNode* node)
{
	GlProgramFactory::RemoveReference(node);
}

#define VP_INVERT_IMAGE_SHADER_VERSION				0

Bool InvertData::GlDraw(BaseVideoPost *node, BaseDraw *bd, GlFrameBuffer* fbuf, LONG colortex, LONG depthtex, VIDEOPOST_GLDRAW flags)
{
	if (flags != VIDEOPOST_GLDRAW_DRAW)
		return FALSE;

	SReal prScale[3];
	VPInvertDescData *pDescData = NULL;
	Bool bFactoryBound = FALSE;
	C4DGLuint nTexture;
	LONG lAttributeCount, lVectorBufferCount;
	const GlVertexBufferAttributeInfo* const* ppAttibuteInfo;
	const GlVertexBufferVectorInfo* const* ppVectorInfo;
	C4D_ALIGN(LONG lIdentity[1], 8);
	lIdentity[0] = VP_INVERT_IMAGE_SHADER_VERSION;

	// get the scale ratios that we don't put the texture on the entire polygon
	Real v1,v2;
	fbuf->GetRatios(C4D_FRAMEBUFFER_COLOR, v1, v2);
	prScale[0] = v1; 
	prScale[1] = v2; 
	if (prScale[0] <= 0.0f || prScale[1] <= 0.0f)
		return FALSE;
	prScale[0] = 1.0f / prScale[0];
	prScale[1] = 1.0f / prScale[1];

	bd->SetDrawParam(DRAW_PARAMETER_USE_Z, FALSE);

	if (!bd->GetFullscreenPolygonVectors(lAttributeCount, ppAttibuteInfo, lVectorBufferCount, ppVectorInfo))
		return FALSE;

	GlProgramFactory* pFactory = GlProgramFactory::GetFactory(bd, node, 0, NULL, lIdentity, sizeof(lIdentity), NULL, 0, 0, ppAttibuteInfo, lAttributeCount, ppVectorInfo, lVectorBufferCount, NULL);
	if (!pFactory)
		return FALSE;

	pFactory->LockFactory();
	if (!pFactory->BindToView(bd))
	{
		pFactory->UnlockFactory();
		goto DisplayError;
	}

	pDescData = (VPInvertDescData*)pFactory->GetDescriptionData(0, 0, InvertData::AllocCgDescription, InvertData::FreeCgDescription, InvertData::ReadCgDescription, InvertData::WriteCgDescription);
	if (!pDescData)
	{
		pFactory->UnlockFactory();
		goto DisplayError;
	}

	if (!pFactory->IsProgram(CompiledProgram))
	{
		// add all necessary parameters
		pFactory->AddParameters(GL_PROGRAM_PARAM_OBJECTCOORD);
		pFactory->Init(0);
		pDescData->strTexsize = pFactory->AddUniformParameter(VertexProgram, UniformFloat2, "texsize");
		pDescData->strTexture = pFactory->AddUniformParameter(FragmentProgram, UniformTexture2D, "texture");
		if (!pFactory->HeaderFinished())
			goto DisplayError;

		// now, add the program source code
		pFactory->AddLine(VertexProgram, "oposition = vec4(iposition.xy, -1.0, 1.0);");
		pFactory->AddLine(VertexProgram, "objectcoord = vec4(.5 * (iposition.xy + vec2(1.0)), 0.0, 0.0);");
		pFactory->AddLine(VertexProgram, "objectcoord.xy = objectcoord.xy * " + pDescData->strTexsize + ".xy;");
		pFactory->AddLine(FragmentProgram, "ocolor.rgb=vec3(1.0)-texture2D(" + pDescData->strTexture + ", objectcoord.xy).rgb;");
		pFactory->AddLine(FragmentProgram, "ocolor.a=1.0;");

		if (!pFactory->CompilePrograms())
		{
			pFactory->DestroyPrograms();
			goto DisplayError;
		}
		pDescData->paramTexsize = pFactory->GetParameterHandle(VertexProgram, pDescData->strTexsize.GetCString());
		pDescData->paramTexture = pFactory->GetParameterHandle(FragmentProgram, pDescData->strTexture.GetCString());
		pFactory->GetVectorInfo(pDescData->lVectorCount, pDescData->ppVectorInfo);
	}
	if (!pFactory->BindPrograms())
	{
		pFactory->UnbindPrograms();
		goto DisplayError;
	}
	bFactoryBound = TRUE;
	pFactory->UnlockFactory();

	// set the program parameters
	pFactory->InitSetParameters();
	pFactory->SetParameterReal2(pDescData->paramTexsize, prScale);
	nTexture = fbuf->GetTexture(colortex, C4D_FRAMEBUFFER_COLOR);
	pFactory->SetParameterTexture(pDescData->paramTexture, 2, nTexture);

	bd->DrawFullscreenPolygon(pDescData->lVectorCount, pDescData->ppVectorInfo);

	pFactory->UnbindPrograms();
	return TRUE;

DisplayError:
	if (pFactory)
	{
		if (bFactoryBound)
			pFactory->UnbindPrograms();
		pFactory->BindToView((BaseDraw*)NULL);
		pFactory->UnlockFactory();
	}
	return FALSE;
}

VIDEOPOST_GLINFO InvertData::GetGlInfo(BaseVideoPost *node, BaseDocument* doc, BaseDraw *bd)
{
	return VIDEOPOST_GLINFO_DRAW;
}

Bool InvertData::RenderEngineCheck(BaseVideoPost *node, LONG id)
{
	// the following render engines are not supported by this effect
	if (id==RDATA_RENDERENGINE_PREVIEWSOFTWARE || 
			id==RDATA_RENDERENGINE_CINEMAN) 
		return FALSE;

	return TRUE; 
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_INVERTVIDEOPOST 1000455

Bool RegisterVPInvertImage(void)
{
	return RegisterVideoPostPlugin(ID_INVERTVIDEOPOST,GeLoadString(IDS_VPINVERTIMAGE),PLUGINFLAG_VIDEOPOST_GL,InvertData::Alloc,"",0,0);
}
