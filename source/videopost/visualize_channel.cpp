/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2010 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

#include "c4d.h"
#include "c4d_symbols.h"
#include "c4d_gl.h"
#include "vpvisualizechannel.h"
#include "c4d_includes.h"

static LONG GetOGLVersion()
{
	static LONG lVersion = -1;
	if (lVersion > 0)
		return lVersion;
	lVersion = GetMachineFeatures().GetLong(OPENGL_VERSION_INT);
	return lVersion;
}

class VisualizeChannelVideoPost : public VideoPostData
{
public:
	static NodeData *Alloc(void) { return gNew VisualizeChannelVideoPost; }

	virtual Bool Init(GeListNode *node);
	virtual void Free(GeListNode* node);

	virtual VIDEOPOST_GLINFO GetGlInfo(BaseVideoPost *node, BaseDocument* doc, BaseDraw *bd);
	virtual Bool GlDraw(BaseVideoPost *node, BaseDraw *bd, GlFrameBuffer* fbuf, LONG colortex, LONG depthtex, VIDEOPOST_GLDRAW flags);

	virtual Bool RenderEngineCheck(BaseVideoPost *node, LONG id);

protected:
	static void* AllocCgDescription();
	static void FreeCgDescription(void* pData);
	static Bool ReadCgDescription(GlReadDescriptionData* pFile, void* pData);
	static Bool WriteCgDescription(GlWriteDescriptionData* pFile, const void* pData);
};

struct VPVisualizeUVWData
{
	GlString strTexsize, strDepthTexture, strColorTexture, strChannelTexture, strStrengthClip;
	GlProgramParameter paramTexsize, paramDepthTexture, paramColorTexture, paramChannelTexture, paramStrengthClip;
	LONG lVectorCount;
	const GlVertexBufferVectorInfo* const* ppVectorInfo;
};

void* VisualizeChannelVideoPost::AllocCgDescription()
{
	return gNew VPVisualizeUVWData;
}

void VisualizeChannelVideoPost::FreeCgDescription(void* pData)
{
	VPVisualizeUVWData* pDescData = (VPVisualizeUVWData*)pData;
	gDelete(pDescData);
}

Bool VisualizeChannelVideoPost::ReadCgDescription(GlReadDescriptionData* pFile, void* pData)
{
	VPVisualizeUVWData* pDesc = (VPVisualizeUVWData*)pData;
	if (!GlProgramFactory::ReadParameter(pFile, pDesc->paramTexsize)) return FALSE;
	if (!GlProgramFactory::ReadParameter(pFile, pDesc->paramDepthTexture)) return FALSE;
	if (!GlProgramFactory::ReadParameter(pFile, pDesc->paramColorTexture)) return FALSE;
	if (!GlProgramFactory::ReadParameter(pFile, pDesc->paramChannelTexture)) return FALSE;
	if (!GlProgramFactory::ReadParameter(pFile, pDesc->paramStrengthClip)) return FALSE;
	return TRUE;
}

Bool VisualizeChannelVideoPost::WriteCgDescription(GlWriteDescriptionData* pFile, const void* pData)
{
	const VPVisualizeUVWData* pDesc = (const VPVisualizeUVWData*)pData;
	if (!GlProgramFactory::WriteParameter(pFile, pDesc->paramTexsize)) return FALSE;
	if (!GlProgramFactory::WriteParameter(pFile, pDesc->paramDepthTexture)) return FALSE;
	if (!GlProgramFactory::WriteParameter(pFile, pDesc->paramColorTexture)) return FALSE;
	if (!GlProgramFactory::WriteParameter(pFile, pDesc->paramChannelTexture)) return FALSE;
	if (!GlProgramFactory::WriteParameter(pFile, pDesc->paramStrengthClip)) return FALSE;
	return TRUE;
}

Bool VisualizeChannelVideoPost::Init(GeListNode *node)
{
	BaseContainer* bc = ((BaseList2D*)node)->GetDataInstance();
	bc->SetReal(VP_VISUALIZE_CHANNEL_STRENGTH, 1.0f);
	bc->SetLong(VP_VISUALIZE_CHANNEL_CHN, VP_VISUALIZE_CHANNEL_CHN_UVW);
	bc->SetReal(VP_VISUALIZE_CHANNEL_LOCLIP, 0.0f);
	bc->SetReal(VP_VISUALIZE_CHANNEL_HICLIP, 1.0f);
	return TRUE;
}

void VisualizeChannelVideoPost::Free(GeListNode* node)
{
	GlProgramFactory::RemoveReference(node);
}

VIDEOPOST_GLINFO VisualizeChannelVideoPost::GetGlInfo(BaseVideoPost *node, BaseDocument* doc, BaseDraw *bd)
{
	BaseContainer* bc = node->GetDataInstance();
	VIDEOPOST_GLINFO ulInfo = VIDEOPOST_GLINFO_DRAW;
	switch (bc->GetLong(VP_VISUALIZE_CHANNEL_CHN))
	{
		default:
		case VP_VISUALIZE_CHANNEL_CHN_UVW: ulInfo |= VIDEOPOST_GLINFO_ALLOC_UVW_BUFFER; break;
		case VP_VISUALIZE_CHANNEL_CHN_DU: ulInfo |= VIDEOPOST_GLINFO_ALLOC_DU_BUFFER; break;
		case VP_VISUALIZE_CHANNEL_CHN_DV: ulInfo |= VIDEOPOST_GLINFO_ALLOC_DV_BUFFER; break;
		case VP_VISUALIZE_CHANNEL_CHN_NORMAL: ulInfo |= VIDEOPOST_GLINFO_ALLOC_NORMAL_BUFFER; break;
		case VP_VISUALIZE_CHANNEL_CHN_DEPTH: break;
		case VP_VISUALIZE_CHANNEL_CHN_MAT_ID: ulInfo |= VIDEOPOST_GLINFO_ALLOC_MATERIAL_ID; break;
		case VP_VISUALIZE_CHANNEL_CHN_WORLD_COORDINATES: ulInfo |= VIDEOPOST_GLINFO_ALLOC_WORLCOORD_BUFFER; break;
		case VP_VISUALIZE_CHANNEL_CHN_OBJECTID: ulInfo |= VIDEOPOST_GLINFO_ALLOC_OBJECTID_BUFFER; break;
	}

	return ulInfo;
}

#define VISUALIZE_CHANNEL_SHADER_VERSION	0

Bool VisualizeChannelVideoPost::GlDraw(BaseVideoPost *node, BaseDraw *bd, GlFrameBuffer* fbuf, LONG colortex, LONG depthtex, VIDEOPOST_GLDRAW flags)
{
	LONG lTexture = NOTOK;
	BaseContainer* bc = node->GetDataInstance();
	LONG lChannel = bc->GetLong(VP_VISUALIZE_CHANNEL_CHN);
	switch (lChannel)
	{
		default:
		case VP_VISUALIZE_CHANNEL_CHN_UVW: lTexture = C4D_FRAMEBUFFER_UV; break;
		case VP_VISUALIZE_CHANNEL_CHN_DU: lTexture = C4D_FRAMEBUFFER_DU; break;
		case VP_VISUALIZE_CHANNEL_CHN_DV: lTexture = C4D_FRAMEBUFFER_DV; break;
		case VP_VISUALIZE_CHANNEL_CHN_NORMAL: lTexture = C4D_FRAMEBUFFER_NORMAL; break;
		case VP_VISUALIZE_CHANNEL_CHN_DEPTH: lTexture = 0; break;
		case VP_VISUALIZE_CHANNEL_CHN_MAT_ID: lTexture = C4D_FRAMEBUFFER_MATERIAL_ID; break;
		case VP_VISUALIZE_CHANNEL_CHN_WORLD_COORDINATES: lTexture = C4D_FRAMEBUFFER_WORLDCOORD; break;
		case VP_VISUALIZE_CHANNEL_CHN_OBJECTID: lTexture = C4D_FRAMEBUFFER_OBJECT_MAT_ID; break;
		break;
	}

	GlFBAdditionalTextureInfo* tex = NULL;
	C4DGLuint nTexture = 0;
	if (lTexture == 0)
	{
		if (lChannel == VP_VISUALIZE_CHANNEL_CHN_DEPTH)
			nTexture = fbuf->GetTexture(depthtex, C4D_FRAMEBUFFER_DEPTH);
	}
	else
	{
		tex = fbuf->GetAdditionalTexture(lTexture);
		if (tex)
			nTexture = tex->nTexture;
	}
	if (nTexture)
	{
		LONG x1, y1, x2, y2;
		BaseContainer* pbcData = node->GetDataInstance();
		bd->GetFrame(&x1, &y1, &x2, &y2);
		x2++;
		y2++;
		SReal prScale[3];
		VPVisualizeUVWData *pDescData = NULL;
		Bool bFactoryBound = FALSE;
		C4DGLuint nDepthTexture, nColorTexture;
		Real rLowClip, rHighClip;
		LONG lAttributeCount, lVectorBufferCount;
		const GlVertexBufferAttributeInfo* const* ppAttibuteInfo;
		const GlVertexBufferVectorInfo* const* ppVectorInfo;

		// get the scale ratios that we don't put the texture on the entire polygon
		Real v1, v2;
		fbuf->GetRatios(C4D_FRAMEBUFFER_COLOR, v1, v2);
		prScale[0] = v1;
		prScale[1] = v2;
		if (prScale[0] <= 0.0f || prScale[1] <= 0.0f)
			return FALSE;
		prScale[2] = 1.0f / prScale[1];
		prScale[0] = 1.0f / prScale[0];
		prScale[1] = 1.0f / prScale[1];

		bd->SetMatrix_Screen();
		bd->SetDrawParam(DRAW_PARAMETER_USE_Z, FALSE);

		Bool bHasIntegerTexture = lChannel == VP_VISUALIZE_CHANNEL_CHN_MAT_ID || lChannel == VP_VISUALIZE_CHANNEL_CHN_OBJECTID;
		C4D_ALIGN(LONG lIdentity[2], 8);
		lIdentity[0] = VISUALIZE_CHANNEL_SHADER_VERSION;
		lIdentity[1] = bHasIntegerTexture ? 1 : 0;
		if (lChannel == VP_VISUALIZE_CHANNEL_CHN_DEPTH)
			lIdentity[1] |= 2;

		if (!bd->GetFullscreenPolygonVectors(lAttributeCount, ppAttibuteInfo, lVectorBufferCount, ppVectorInfo))
			return FALSE;

		GlProgramFactory* pFactory = GlProgramFactory::GetFactory(bd, (C4DAtom*)node, 0, NULL, lIdentity, sizeof(lIdentity), NULL, 0, GL_GET_PROGRAM_FACTORY_ALLOW_SHARING, ppAttibuteInfo, lAttributeCount, ppVectorInfo, lVectorBufferCount, NULL);
		if (!pFactory)
			return FALSE;

		pFactory->LockFactory();
		if (!pFactory->BindToView(bd))
		{
			pFactory->UnlockFactory();
			goto DisplayError;
		}

		pDescData = (VPVisualizeUVWData*)pFactory->GetDescriptionData(0, 0, VisualizeChannelVideoPost::AllocCgDescription, VisualizeChannelVideoPost::FreeCgDescription,
			VisualizeChannelVideoPost::ReadCgDescription, VisualizeChannelVideoPost::WriteCgDescription);
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
			pDescData->strTexsize = pFactory->AddUniformParameter(VertexProgram, UniformFloat3, "texsize");
			if (bHasIntegerTexture)
				pDescData->strChannelTexture = pFactory->AddUniformParameter(FragmentProgram, UniformTexture2Du, "channeltex");
			else
				pDescData->strChannelTexture = pFactory->AddUniformParameter(FragmentProgram, UniformTexture2D, "channeltex");
			pDescData->strDepthTexture = pFactory->AddUniformParameter(FragmentProgram, UniformTexture2D, "depthtex");
			pDescData->strColorTexture = pFactory->AddUniformParameter(FragmentProgram, UniformTexture2D, "colortex");
			pDescData->strStrengthClip = pFactory->AddUniformParameter(FragmentProgram, UniformFloat3, "strength_clip");
			if (!pFactory->HeaderFinished())
				goto DisplayError;

			// now, add the program source code
			GlString strMix = "mix" + pFactory->GetUniqueID();
			pFactory->AddLine(VertexProgram, "oposition = vec4(iposition, 0.0, 1.0);");
			pFactory->AddLine(VertexProgram, "objectcoord.xy = iposition.xy * .5 + vec2(.5);");
			pFactory->AddLine(VertexProgram, "objectcoord.xy *= " + pDescData->strTexsize + ".xy;");
			pFactory->AddLine(VertexProgram, "objectcoord.zw = vec2(0.0);");
			pFactory->AddLine(FragmentProgram, "#define DEPTH_DELTA		0.999999");
			pFactory->AddLine(FragmentProgram, "vec2 uv = objectcoord.xy;");
			if (GetOGLVersion() >= 300)
				pFactory->AddLine(FragmentProgram, "float " + strMix + " = texture2D(" + pDescData->strDepthTexture + ", uv).r >= DEPTH_DELTA ? 0.0 : " + pDescData->strStrengthClip + ".x;");
			else
				pFactory->AddLine(FragmentProgram, "float " + strMix + " = texture2D(" + pDescData->strDepthTexture + ", uv).a >= DEPTH_DELTA ? 0.0 : " + pDescData->strStrengthClip + ".x;");
			if (lChannel == VP_VISUALIZE_CHANNEL_CHN_MAT_ID)
			{
				pFactory->AddLine(FragmentProgram, "unsigned int n = texture2D(" + pDescData->strChannelTexture + ", uv).r;");
				pFactory->AddLine(FragmentProgram, "vec3 rgb = vec3(float((n >> 16u) & 255u), float((n >> 8u) & 255u), float(n & 255u)) / 255.0;");
				pFactory->AddLine(FragmentProgram, "ocolor.rgb=mix(texture2D(" + pDescData->strColorTexture + ", uv).rgb, (rgb - " + pDescData->strStrengthClip + ".yyy) * " + pDescData->strStrengthClip + ".zzz, " + strMix + ");");
			}
			else if (lChannel == VP_VISUALIZE_CHANNEL_CHN_OBJECTID)
			{
				pFactory->AddLine(FragmentProgram, "unsigned int n = texture2D(" + pDescData->strChannelTexture + ", uv).r;");
				pFactory->AddLine(FragmentProgram, "vec3 rgb = vec3(float((n >> 24u) & 255u), float((n >> 16u) & 255u), float((n >> 8u) & 255u)) / 255.0;");
				pFactory->AddLine(FragmentProgram, "ocolor.rgb=mix(texture2D(" + pDescData->strColorTexture + ", uv).rgb, (rgb - " + pDescData->strStrengthClip + ".yyy) * " + pDescData->strStrengthClip + ".zzz, " + strMix + ");");
			}
			else if (lChannel == VP_VISUALIZE_CHANNEL_CHN_DEPTH)
			{
				if (GetOGLVersion() >= 300)			
					pFactory->AddLine(FragmentProgram, "ocolor.rgb=mix(texture2D(" + pDescData->strColorTexture + ", uv).rgb, (texture2D(" + pDescData->strChannelTexture + ", uv).rrr - " + pDescData->strStrengthClip + ".yyy) * " + pDescData->strStrengthClip + ".zzz, " + strMix + ");");
				else
					pFactory->AddLine(FragmentProgram, "ocolor.rgb=mix(texture2D(" + pDescData->strColorTexture + ", uv).rgb, (texture2D(" + pDescData->strChannelTexture + ", uv).aaa - " + pDescData->strStrengthClip + ".yyy) * " + pDescData->strStrengthClip + ".zzz, " + strMix + ");");
			}
			else
				pFactory->AddLine(FragmentProgram, "ocolor.rgb=mix(texture2D(" + pDescData->strColorTexture + ", uv).rgb, (texture2D(" + pDescData->strChannelTexture + ", uv).rgb - " + pDescData->strStrengthClip + ".yyy) * " + pDescData->strStrengthClip + ".zzz, " + strMix + ");");
			pFactory->AddLine(FragmentProgram, "ocolor.a=1.0;");

			if (!pFactory->CompilePrograms())
			{
				pFactory->DestroyPrograms();
				goto DisplayError;
			}

			//pDescData->paramTransmatrix = pFactory->GetParameterHandle(VertexProgram, "transmatrix");
			pDescData->paramTexsize = pFactory->GetParameterHandle(VertexProgram, pDescData->strTexsize.GetCString());
			pDescData->paramDepthTexture = pFactory->GetParameterHandle(FragmentProgram, pDescData->strDepthTexture.GetCString());
			pDescData->paramColorTexture = pFactory->GetParameterHandle(FragmentProgram, pDescData->strColorTexture.GetCString());
			pDescData->paramChannelTexture = pFactory->GetParameterHandle(FragmentProgram, pDescData->strChannelTexture.GetCString());
			pDescData->paramStrengthClip = pFactory->GetParameterHandle(FragmentProgram, pDescData->strStrengthClip.GetCString());

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
		//pFactory->SetParameterMatrixTransform(pDescData->paramTransmatrix);
		pFactory->SetParameterReal3(pDescData->paramTexsize, prScale);
		nDepthTexture = fbuf->GetTexture(depthtex, C4D_FRAMEBUFFER_DEPTH);
		nColorTexture = fbuf->GetTexture(colortex, C4D_FRAMEBUFFER_COLOR);
		pFactory->SetParameterTexture2DDepth(pDescData->paramDepthTexture, nDepthTexture);
		pFactory->SetParameterTexture(pDescData->paramColorTexture, 2, nColorTexture);
		if (lChannel == VP_VISUALIZE_CHANNEL_CHN_DEPTH)
			pFactory->SetParameterTexture2DDepth(pDescData->paramChannelTexture, nTexture);
		else
			pFactory->SetParameterTexture(pDescData->paramChannelTexture, 2, nTexture);
		rLowClip = pbcData->GetReal(VP_VISUALIZE_CHANNEL_LOCLIP);
		rHighClip = pbcData->GetReal(VP_VISUALIZE_CHANNEL_HICLIP);
		if (lChannel == VP_VISUALIZE_CHANNEL_CHN_WORLD_COORDINATES)
		{
			rHighClip *= 100.0f;
			rLowClip *= 100.0f;
		}
		rHighClip -= rLowClip;
		if (rHighClip > 0.00001f)
			rHighClip = 1.0f / rHighClip;
		pFactory->SetParameterReal3(pDescData->paramStrengthClip, pbcData->GetReal(VP_VISUALIZE_CHANNEL_STRENGTH), rLowClip, rHighClip);

		// draw a single full-screen polygon
		bd->DrawFullscreenPolygon(pDescData->lVectorCount, pDescData->ppVectorInfo);

		pFactory->UnbindPrograms();

		//fbuf->DrawBuffer(C4D_FRAMEBUFFER_TARGET_TEX_ADDITIONAL, C4D_FRAMEBUFFER_UV);

		return TRUE;

DisplayError:
		if (pFactory && bFactoryBound)
		{
			pFactory->UnbindPrograms();
		}
		pFactory->UnlockFactory();
		return FALSE;
	}
	else
		fbuf->DrawBuffer(colortex, C4D_FRAMEBUFFER_COLOR);
	return TRUE;
}

Bool VisualizeChannelVideoPost::RenderEngineCheck(BaseVideoPost *node, LONG id)
{
	// the following render engines are not supported by this effect
	if (id!=RDATA_RENDERENGINE_PREVIEWHARDWARE) 
		return FALSE;

	return TRUE; 
}

#define ID_VISUALIZE_CHANNEL_VIDEOPOST 450000227
Bool RegisterVPVisualizeChannel(void)
{
	// decide by name if the plugin shall be registered - just for user convenience
	String name = GeLoadString(IDS_VPVISUALIZE_CHANNEL);
	if (!name.Content())
		return TRUE;
	return RegisterVideoPostPlugin(ID_VISUALIZE_CHANNEL_VIDEOPOST, name, PLUGINFLAG_VIDEOPOST_GL, VisualizeChannelVideoPost::Alloc, "vpvisualizechannel", 0, 0);
}
