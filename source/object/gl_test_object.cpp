// In this example we show how to draw certain objects with OpenGL and enhanced OpenGL.

#include "c4d.h"
#include "ogltest.h"
#include "c4d_symbols.h"
#include "c4d_gl.h"
#include "dbasedraw.h"

class GLTestObject : public ObjectData
{
public:
	static NodeData *Alloc(void) { return gNew GLTestObject; }

protected:
	virtual Bool Init(GeListNode *node);
	virtual void Free(GeListNode *node);
	virtual DRAWRESULT Draw(BaseObject *op, DRAWPASS type, BaseDraw *bd, BaseDrawHelp *bh);

	Bool DrawParticles(BaseObject* pObject, BaseDraw *bd, BaseDrawHelp *bh, LONG lType);

	static SVector *g_pvParticlePoint, *g_pvParticleColor, *g_pvParticleNormal;
	GlVertexSubBufferData* m_pSubBuffer;

public:
	static void FreeParticleData();
};

SVector* GLTestObject::g_pvParticlePoint = NULL;
SVector* GLTestObject::g_pvParticleColor = NULL;
SVector* GLTestObject::g_pvParticleNormal = NULL;

static GlVertexBufferVectorInfo *g_pVectorInfo = NULL, *g_ppVectorInfo[3];
static GlVertexBufferAttributeInfo* g_pAttributeInfo = NULL, *g_ppAttributeInfo[3];

void GLTestObject::FreeParticleData()
{
	GeFree(g_pvParticlePoint);
	GeFree(g_pvParticleColor);
	GeFree(g_pvParticleNormal);
}

Bool GLTestObject::Init(GeListNode *node)
{
	BaseObject		*pObject = (BaseObject*)node;
	BaseContainer *pbcData = pObject->GetDataInstance();

	pbcData->SetLong(GLTEST_TYPE, GLTEST_TYPE_PARTICLE_FAST_BUFFER);

	return TRUE;
}

void GLTestObject::Free(GeListNode *node)
{
	GlVertexBuffer::RemoveObject((C4DAtom*)node, 0);
	GlProgramFactory::RemoveReference((C4DAtom*)node);
}

DRAWRESULT GLTestObject::Draw(BaseObject *op, DRAWPASS type, BaseDraw *bd, BaseDrawHelp *bh)
{
	if (type != DRAWPASS_OBJECT)
		return DRAWRESULT_SKIP;

	BaseContainer* pbcData = op->GetDataInstance();
	LONG lType = pbcData->GetLong(GLTEST_TYPE);
	return DrawParticles(op, bd, bh, lType) ? DRAWRESULT_OK : DRAWRESULT_SKIP;
}

SReal value(SReal nl, SReal n2, SReal hue)
{
	if (hue > 360)
		hue -= 360;
	else if (hue < 0)
		hue += 360;
	if (hue < 60)
		return nl + (n2 - nl) * hue / 60.0;
	if (hue < 180)
		return n2;
	if (hue < 240)
		return nl + (n2 - nl) * (240 - hue) / 60.0;
	return nl;
} 

void HLStoRGB(SReal h, SReal l, SReal s, SReal *r, SReal *g, SReal *b)
{ 
	SReal m1, m2;
	if (l <= 0.5)
		m2 = l * (1.0 + s);
	else
		m2 = l + s - l * s;
	m1 = 2.0 * l - m2; 
	if (s == 0 || h == -1)
		*r = *g = *b = l;
	else
	{
		*r = value(m1, m2, h + 120);
		*g = value(m1, m2, h);
		*b = value(m1, m2, h - 120);
	} 
}
Bool GLTestObject::DrawParticles(BaseObject* pObject, BaseDraw *bd, BaseDrawHelp *bh, LONG lType)
{
	const LONG lPoints = 1000000;
	LONG l;
	SVector v(DC);
	SReal r, rRad, x, z, n;
	Matrix mg;
	BaseDocument* pDoc = bh->GetDocument();
	GlVertexBufferDrawSubbuffer di(C4D_VERTEX_BUFFER_POINTS, lPoints, 0);
	LONG lVectorCount;
	const GlVertexBufferVectorInfo* const* ppVectorInfo;
	const GlLight** ppLights = NULL;
	GlProgramFactory* pFactory = NULL;
	LONG lLightCount = 0;

	LONG lDrawportType = bd->GetDrawParam(BASEDRAW_DRAWPORTTYPE).GetLong();
	if (lDrawportType != DRAWPORT_TYPE_OGL_HQ)
		return TRUE;

	if (!g_pvParticlePoint || !g_pvParticleColor || !g_pvParticleNormal)
	{
		GeFree(g_pvParticlePoint);
		GeFree(g_pvParticleColor);
		GeFree(g_pvParticleNormal);

		// prepare the point array
		g_pvParticlePoint = GeAllocTypeNC(SVector,lPoints);
		g_pvParticleColor = GeAllocTypeNC(SVector,lPoints);
		g_pvParticleNormal = GeAllocTypeNC(SVector,lPoints);
		if (!g_pvParticlePoint || !g_pvParticleColor || !g_pvParticleNormal)
			return FALSE;

		// make a disc with radius from 0 to 100 and let the radius grow each time
		v.z = 0.0f;
		for (l = 0; l < lPoints; l++)
		{
			// point
			r = (Real)l / (Real)lPoints;
			rRad = r * 100.0f;
			v.x = Cos(r * pi2 * 300.0f) * rRad;
			v.y = Sin(r * pi2 * 300.0f) * rRad;
			g_pvParticlePoint[l] = v;

			// color
			HLStoRGB(Modulo(r * 300.0f * 360.0, 360.0), 1.0f - .5f * r, 1.0f, &g_pvParticleColor[l].x, &g_pvParticleColor[l].y, &g_pvParticleColor[l].z);

			// normal
			n = v.x / 50.0f;
			n = Cos(n * pi2) * Rad(20.0f);
			z = Cos(n);
			x = Sin(n);

			g_pvParticleNormal[l] = SVector(x, 0.0, z);
		}
	}

	mg = pObject->GetMg() * MatrixRotZ(pDoc->GetTime().Get() * pi2);

	if (lType == GLTEST_TYPE_PARTICLE_FAST_BLOCK || lType == GLTEST_TYPE_PARTICLE_FAST_BUFFER)
	{
		Bool bEOGL = FALSE;
		if (lDrawportType == DRAWPORT_TYPE_OGL_HQ && lType == GLTEST_TYPE_PARTICLE_FAST_BUFFER)
		{
			GlVertexBuffer* pBuffer = GlVertexBuffer::GetVertexBuffer(bd, pObject, 0, NULL, 0, 0);
			if (!pBuffer)
				goto _no_eogl;
			if (pBuffer->IsDirty())
			{
				// let's allocate a buffer that holds our data
				m_pSubBuffer = pBuffer->AllocSubBuffer(bd, VBArrayBuffer, lPoints * (sizeof(SVector) + sizeof(SVector) + sizeof(SVector)), NULL);
				if (!m_pSubBuffer)
					goto _no_eogl;
				// map the buffer so that we can store our data
				// note that this memory is located on the graphics card
				void* pData = pBuffer->MapBuffer(bd, m_pSubBuffer, VBWriteOnly);
				if (!pData)
				{
					pBuffer->UnmapBuffer(bd, m_pSubBuffer);
					goto _no_eogl;
				}
				SVector* pvData = (SVector*)pData;
				for (l = 0; l < lPoints; l++)
				{
					*pvData++ = g_pvParticlePoint[l];
					*pvData++ = g_pvParticleNormal[l];
					*pvData++ = g_pvParticleColor[l];
				}
				pBuffer->UnmapBuffer(bd, m_pSubBuffer);
				pBuffer->SetDirty(FALSE);
			}

			lLightCount = bd->GetGlLightCount();
			if (lLightCount > 0)
			{
				ppLights = GeAllocType(const GlLight*, lLightCount);
				for (l = 0; l < lLightCount; l++)
					ppLights[l] = bd->GetGlLight(l);
			}
			else
				ppLights = 0;

			// we need a program to show the data
			pFactory = GlProgramFactory::GetFactory(bd, pObject, 0, NULL, NULL, 0, ppLights, lLightCount, 0, g_ppAttributeInfo, 3, g_ppVectorInfo, 3, NULL);
			if (!pFactory)
				goto _no_eogl;
			pFactory->LockFactory();
			pFactory->BindToView(bd);
			if (!pFactory->IsProgram(CompiledProgram))
			{
				// just route the vertex information to the fragment program
				ULONG ulParameters = GL_PROGRAM_PARAM_NORMALS | GL_PROGRAM_PARAM_COLOR;
				if (lLightCount > 0)
					ulParameters |= GL_PROGRAM_PARAM_EYEPOSITION;
				pFactory->AddParameters(ulParameters);
				pFactory->Init(0);
				pFactory->HeaderFinished();
				pFactory->AddLine(VertexProgram, "oposition = (transmatrix * vec4(iposition.xyz, 1.0));");
				pFactory->AddLine(VertexProgram, "ocolor = vec4(icolor.rgb, 1.0);");
				pFactory->AddLine(VertexProgram, "onormal = vec4(inormal.xyz, 1.0);");
				if (lLightCount)
				{
					pFactory->AddLine(VertexProgram, "worldcoord.xyz = objectmatrix_r * iposition.xyz + objectmatrix_p.xyz;");
					pFactory->AddLine(FragmentProgram, "vec3 V = normalize(eyeposition - worldcoord.xyz);");
					pFactory->AddLine(FragmentProgram, "ocolor = vec4(0.0);");
					pFactory->StartLightLoop();
					pFactory->AddLine(FragmentProgram, "ocolor.rgb += icolor.rgb * lightcolorD.rgb * abs(dot(normal.xyz, L.xyz));");
					pFactory->EndLightLoop();
					pFactory->AddLine(FragmentProgram, "ocolor.a = 1.0;");
				}
				else
				{
					// use the fragment color if no light source is used
					pFactory->AddLine(FragmentProgram, "ocolor = icolor;");
				}
				pFactory->CompilePrograms();
				if (lLightCount > 0)
					pFactory->InitLightParameters();
			}

			bd->SetMatrix_Matrix(NULL, Matrix());
			pFactory->BindPrograms();
			pFactory->SetParameterMatrixTransform(pFactory->GetParameterHandle(VertexProgram, "transmatrix"));
			if (lLightCount > 0)
			{
				pFactory->SetLightParameters(ppLights, lLightCount, SMatrix4());
				pFactory->SetParameterVector(pFactory->GetParameterHandle(FragmentProgram, "eyeposition"), bd->GetMg().off.ToSV());
				pFactory->SetParameterMatrix(pFactory->GetParameterHandle(FragmentProgram, "objectmatrix_p"), pFactory->GetParameterHandle(FragmentProgram, "objectmatrix_r"), -1, SMatrix());
			}
			// obtain the vector information from the factory so that we have the proper attribute locations
			pFactory->GetVectorInfo(lVectorCount, ppVectorInfo);
			pBuffer->DrawSubBuffer(bd, m_pSubBuffer, NULL, 1, &di, lVectorCount, ppVectorInfo);
			pFactory->UnbindPrograms();

			pFactory->BindToView(NULL);
			pFactory->UnlockFactory();

			bEOGL = TRUE;
_no_eogl: ;
		}
		if (!bEOGL)
		{
			// this will copy all data into a buffer and draw them immediately
			bd->DrawPointArray(lPoints, g_pvParticlePoint, &(g_pvParticleColor->x), 3, g_pvParticleNormal);
		}
	}
	else
	{
		// this will copy each point into an internal buffer and draw the buffer when a certain limit has been reached
		bd->SetTransparency(0);
		for (l = 0; l < lPoints; l++)
		{
			bd->SetPen(g_pvParticleColor[l].ToRV());
			//bd->Point3D(g_pvParticlePoint[l].ToRV());
		}
	}

	return TRUE;
}

Bool RegisterGLTestObject()
{
	String name = GeLoadString(IDS_GL_TEST_OBJECT); if (!name.Content()) return TRUE;

	g_pVectorInfo = bNew GlVertexBufferVectorInfo[3];
	g_pAttributeInfo = bNew GlVertexBufferAttributeInfo[3];
	if (!g_pVectorInfo || !g_pAttributeInfo)
		return FALSE;

	for (LONG a = 0; a < 3; a++)
	{
		g_ppVectorInfo[a] = &(g_pVectorInfo[a]);
		g_ppAttributeInfo[a] = &(g_pAttributeInfo[a]);
	}

	VLONG lStride = 3 * sizeof(SVector);
	g_pVectorInfo[0].lDataType = C4D_GL_DATATYPE_FLOAT;
	g_pVectorInfo[0].lCount = 3;
	g_pVectorInfo[0].strType = "vec3";
	g_pVectorInfo[0].strName = "attrib0";
	g_pVectorInfo[0].lOffset = 0;
	g_pVectorInfo[0].lStride = lStride;
	g_pVectorInfo[0].nAttribLocation = C4D_VERTEX_BUFFER_VERTEX;

	g_pVectorInfo[1].lDataType = C4D_GL_DATATYPE_FLOAT;
	g_pVectorInfo[1].lCount = 3;
	g_pVectorInfo[1].strType = "vec3";
	g_pVectorInfo[1].strName = "attrib1";
	g_pVectorInfo[1].lOffset = sizeof(SVector);
	g_pVectorInfo[1].lStride = lStride;
	g_pVectorInfo[1].nAttribLocation = C4D_VERTEX_BUFFER_NORMAL;

	g_pVectorInfo[2].lDataType = C4D_GL_DATATYPE_FLOAT;
	g_pVectorInfo[2].lCount = 3;
	g_pVectorInfo[2].strType = "vec3";
	g_pVectorInfo[2].strName = "attrib2";
	g_pVectorInfo[2].lOffset = 2 * sizeof(SVector);
	g_pVectorInfo[2].lStride = lStride;
	g_pVectorInfo[2].nAttribLocation = C4D_VERTEX_BUFFER_COLOR;

	g_pAttributeInfo[0].lVector[0] = g_pAttributeInfo[0].lVector[1] = g_pAttributeInfo[0].lVector[2] = 0;
	g_pAttributeInfo[0].lIndex[0] = 0; g_pAttributeInfo[0].lIndex[1] = 1; g_pAttributeInfo[0].lIndex[2] = 2;
	g_pAttributeInfo[0].strDeclaration = "vec3 iposition = attrib0.xyz";
	g_pAttributeInfo[0].strName = "iposition";

	g_pAttributeInfo[1].lVector[0] = g_pAttributeInfo[1].lVector[1] = g_pAttributeInfo[1].lVector[2] = 0;
	g_pAttributeInfo[1].lIndex[0] = 0; g_pAttributeInfo[1].lIndex[1] = 1; g_pAttributeInfo[1].lIndex[2] = 2;
	g_pAttributeInfo[1].strDeclaration = "vec3 inormal = attrib1.xyz";
	g_pAttributeInfo[1].strName = "inormal";

	g_pAttributeInfo[2].lVector[0] = g_pAttributeInfo[2].lVector[1] = g_pAttributeInfo[2].lVector[2] = 0;
	g_pAttributeInfo[2].lIndex[0] = 0; g_pAttributeInfo[2].lIndex[1] = 1; g_pAttributeInfo[2].lIndex[2] = 2;
	g_pAttributeInfo[2].strDeclaration = "vec3 icolor = attrib2.xyz";
	g_pAttributeInfo[2].strName = "icolor";

	return RegisterObjectPlugin(450000131, name, 0, GLTestObject::Alloc, "ogltest", NULL, 0);
}

void FreeGLTestObject()
{
	bDelete(g_pVectorInfo);
	bDelete(g_pAttributeInfo);

	GLTestObject::FreeParticleData();
}
