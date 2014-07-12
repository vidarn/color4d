#include "palettedialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"
PaletteDialog::PaletteDialog()
{
}


Bool PaletteDialog::CreateLayout(void)
{
    if (!GeDialog::CreateLayout()) return FALSE;
	SetTitle(GeLoadString(IDS_COLORPICKER));
	GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
	if(AddSubDialog(IDC_PALETTE,BFH_SCALEFIT)){
		AttachSubDialog(&m_paletteSubDiag,IDC_PALETTE);
	}
    GroupEnd();
    return TRUE;
}

PaletteDialog::~PaletteDialog()
{
}

LONG PaletteCommand::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}

Bool PaletteCommand::Execute(BaseDocument *doc)
{
	return dlg.Open(DLG_TYPE_ASYNC,PALETTE_ID,-1,-1);
}

Bool PaletteCommand::RestoreLayout(void *secret)
{
	return dlg.RestoreLayout(PALETTE_ID,0,secret);
}