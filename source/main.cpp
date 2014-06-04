#include "colorpickerdialog.h"
#include "colorWheel.h"
#include "colorbox.h"
#include "c4d_symbols.h"
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
    return TRUE;
}

void PluginEnd(void)
{
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