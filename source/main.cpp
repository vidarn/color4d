#include "colorpickerdialog.h"
#include "colorselectordialog.h"
#include "colorWheel.h"
#include "colorbox.h"
#include "c4d_symbols.h"
#include "spotcolordialog.h"
#include "palettedialog.h"
#include "main.h"


Bool IsKeyDown(LONG key)
{
    BaseContainer res;
    return GetInputState(BFM_INPUT_KEYBOARD,key,res) && res.GetLong(BFM_INPUT_VALUE);
}

typedef Bool CChooser(Vector *color, LONG);
CChooser *g_ColorPicker=NULL;

LONG g_InUse=0;

Bool ColorPicker(Vector *color, LONG flags)
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

//////////////////////////////////////////////////////////////////////////

Bool PluginStart(void)
{
	Color::LoadICCProfiles();
	Color::SetWheelProfile(0);
	Color::SetRGBProfile(0);
	Color::SetCMYKProfile(0);
	Color::SetDisplayProfile(0);
	Color::UpdateTransforms();
	ColorScheme::Init();
	PaletteColor::LoadIcons();
	Palette::InitPalettes();
	Bool result = RegisterCommandPlugin(SPOTCOLOR_ID,String("Spot colors"),0,NULL,String(),gNew SpotColorCommand);
	result = result && RegisterCommandPlugin(COLORSELECTOR_ID,String("Color wheel"),0,NULL,String(),gNew ColorSelectorCommand);
	result = result && RegisterCommandPlugin(PALETTE_ID,String("Palette"),0,NULL,String(),gNew PaletteCommand);
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

Bool PluginMessage(LONG id, void *data)
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
