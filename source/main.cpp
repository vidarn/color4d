#include "colorpickerdialog.h"
#include "colorselectordialog.h"
#include "colorWheel.h"
#include "colorbox.h"
#include "c4d_symbols.h"
#include "palettedialog.h"
#include "paletteSceneHook.h"
#include "paletteshader.h"
#include "main.h"

void ErrorHandlerFunction(cmsContext ContextID, cmsUInt32Number ErrorCode, const char *Text){
    printf("Error %u %s\n", ErrorCode, Text);
}


Bool IsKeyDown(Int32 key)
{
    BaseContainer res;
    return GetInputState(BFM_INPUT_KEYBOARD,key,res) && res.GetInt32(BFM_INPUT_VALUE);
}

typedef Bool CChooser(Vector *color, Int32);
CChooser *g_ColorPicker=NULL;

Int32 g_InUse=0;

Bool ColorPicker(Vector *color, Int32 flags)
{
    if (IsKeyDown(KEY_SHIFT))
    {
        if (g_InUse==0 && g_ColorPicker)
        {
            g_InUse=2;

            Bool bRet=g_ColorPicker(color,flags);

            g_InUse=0;

            return bRet;
        }
    }
    else if (g_InUse==0)
    {
        g_InUse=1;

        ColorPickerDialog dlg(color);
        dlg.Open(DLG_TYPE_MODAL_RESIZEABLE,COLORPICKER_ID);

        g_InUse=0;

        return TRUE;
    }
    else if (g_InUse==1 && g_ColorPicker)
    {
        g_InUse=2;

        Bool bRet=g_ColorPicker(color,flags);

        g_InUse=1;

        return bRet;
    }

    return FALSE;
}

class EnableCommand : public CommandData
{
private:
    ColorSelectorDialog dlg;
public:
    virtual Bool Execute(BaseDocument *doc);
    virtual Int32 GetState(BaseDocument *doc);
};

Int32 EnableCommand::GetState(BaseDocument *doc)
{
    Int32 ret = CMD_ENABLED;
    if(!g_InUse) ret = ret | CMD_VALUE;
	return ret;
}

Bool EnableCommand::Execute(BaseDocument *doc)
{
    g_InUse = !g_InUse;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

Bool PluginStart(void)
{
    cmsSetLogErrorHandler(&ErrorHandlerFunction);
	Color::LoadICCProfiles();
	Color::SetWheelProfile(WHEEL_TYPE_HSV);
	Color::SetRGBProfile(0);
	Color::SetCMYKProfile(0);
	Color::SetDisplayProfile(0);
	Color::UpdateTransforms();
	ColorScheme::Init();
	PaletteColor::LoadIcons();
    Palette::InitPalettes();
	Bool result = RegisterCommandPlugin(COLORSELECTOR_ID,String("Color wheel"),0,NULL,String(),NewObjClear(ColorSelectorCommand));
	result = result && RegisterCommandPlugin(PALETTE_ID, String("Palette 1"),0,NULL,String(),NewObjClear(PaletteCommand));
    result = result && RegisterCommandPlugin(PALETTE2_ID,String("Palette 2"),0,NULL,String(),NewObjClear(PaletteCommand2));
    result = result && RegisterCommandPlugin(PALETTE3_ID,String("Palette 3"),0,NULL,String(),NewObjClear(PaletteCommand3));
    result = result && RegisterCommandPlugin(COLORPICKER_ENABLE_ID, String("Use Color Picker"), 0, NULL, String(), NewObjClear(EnableCommand));
    result = result && RegisterPaletteShader();
    result = result && Register_PaletteSceneHook();
	if(result){
		GePrint("Result!");
	}
	else{
		GePrint("Not result!");
	}
	return result;
}

void PluginEnd(void)
{
	PaletteColor::UnloadIcons();
	ColorScheme::Free();
	Color::Unload();
}

Bool PluginMessage(Int32 id, void *data)
{
    switch (id)
    {
    case C4DPL_INIT_SYS:
        {
            if(!resource.Init()) return FALSE;

            OperatingSystem *os=(OperatingSystem*)data;

            g_ColorPicker = os->Ge->ChooseColor;
            os->Ge->ChooseColor = ColorPicker;
        }
        return TRUE;

    case C4DMSG_PRIORITY: 
        return TRUE;
    }

    return FALSE;
}
