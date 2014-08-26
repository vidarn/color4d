#include "palettedialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"
PaletteDialog::PaletteDialog(Int32 id):m_paletteSubDiag(id)
{
}


Bool PaletteDialog::CreateLayout(void)
{
    if (!GeDialog::CreateLayout()) return FALSE;
	SetTitle(GeLoadString(IDS_COLORPICKER));
	GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
	if(AddSubDialog(IDC_PALETTE,BFH_SCALEFIT|BFV_SCALEFIT)){
		AttachSubDialog(&m_paletteSubDiag,IDC_PALETTE);
	}
    GroupEnd();
    return TRUE;
}

PaletteDialog::~PaletteDialog()
{
}

Int32 PaletteCommand::GetState(BaseDocument *doc)
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

Int32 PaletteCommand2::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}

Bool PaletteCommand2::Execute(BaseDocument *doc)
{
	return dlg.Open(DLG_TYPE_ASYNC,PALETTE2_ID,-1,-1);
}

Bool PaletteCommand2::RestoreLayout(void *secret)
{
	return dlg.RestoreLayout(PALETTE2_ID,0,secret);
}

Int32 PaletteCommand3::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}

Bool PaletteCommand3::Execute(BaseDocument *doc)
{
	return dlg.Open(DLG_TYPE_ASYNC,PALETTE3_ID,-1,-1);
}

Bool PaletteCommand3::RestoreLayout(void *secret)
{
	return dlg.RestoreLayout(PALETTE3_ID,0,secret);
}
