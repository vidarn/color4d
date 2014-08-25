#pragma once
#include "c4d.h"
#include "xmandelbrot.h"

class PaletteShaderData : public ShaderData
{
public:
	Vector m_color;
    
public:
	virtual Bool Init		(GeListNode* node);
	virtual	Vector Output(BaseShader* chn, ChannelData* cd);
    virtual Bool Message(GeListNode* node, Int32 type, void* data);
    
	virtual	INITRENDERRESULT InitRender(BaseShader* sh, const InitRenderStruct& irs);
	virtual	void FreeRender(BaseShader* sh);
    
	static NodeData* Alloc(void) { return NewObjClear(PaletteShaderData); }
    
};

Bool RegisterPaletteShader();