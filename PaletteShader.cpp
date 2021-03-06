#include "paletteshader.h"
#include "main.h"
#include "palette.h"
#include "xpalette.h"

Bool PaletteShaderData::Init(GeListNode* node)
{
	BaseContainer* data = ((BaseShader*)node)->GetDataInstance();
    data->SetInt32(PALETTESHADER_PALETTE_ID, 1);
    data->SetInt32(PALETTESHADER_COLOR_ID, 1);
	return true;
}

INITRENDERRESULT PaletteShaderData::InitRender(BaseShader* sh, const InitRenderStruct& irs)
{
	BaseContainer* data = sh->GetDataInstance();
    Int32 paletteId = ClampValue(data->GetInt32(PALETTESHADER_PALETTE_ID),1,99);
    Int32 colorId   = ClampValue(data->GetInt32(PALETTESHADER_COLOR_ID),1,999999);
    Color col;
	Palette::GetPaletteColor(paletteId-1, colorId-1, col);
    m_color = TransformColor(col.Convert(COLOR_SOURCE_DISPLAY).AsVector(),COLORSPACETRANSFORMATION_SRGB_TO_LINEAR);
	return INITRENDERRESULT_OK;
}

void PaletteShaderData::FreeRender(BaseShader* sh)
{
}

Vector PaletteShaderData::Output(BaseShader* chn, ChannelData* cd)
{
	return m_color;
}

Bool PaletteShaderData::Message(GeListNode* node, Int32 type, void* data)
{
    if(type == PALETTE_ID){
        BaseContainer* bc = ((BaseShader*)node)->GetDataInstance();
        Int32 colorId = bc->GetInt32(PALETTESHADER_COLOR_ID)-1;
        Int32 paletteId = bc->GetInt32(PALETTESHADER_PALETTE_ID) - 1;
        ReorderPaletteData *rData = static_cast<ReorderPaletteData *>(data);
        if(rData->newIDs == nullptr){
            if(rData->colorID == colorId && rData->paletteID == paletteId){
                rData->mat->Update(TRUE, TRUE);
            }
        } else {
            if(rData->paletteID == paletteId){
                colorId = ClampValue((Int32)colorId, (Int32)0, (Int32)rData->newIDs->GetCount());
                colorId = (*(rData->newIDs))[colorId] + 1;
                bc->SetInt32(PALETTESHADER_COLOR_ID, colorId);
            }
        }
    }
    return ShaderData::Message(node, type, data);
}

Bool RegisterPaletteShader(void)
{
	return RegisterShaderPlugin(PALETTE_SHADER_ID, String("Color4D Palette Shader"), 0, PaletteShaderData::Alloc, "Xpalette", 0);
}
