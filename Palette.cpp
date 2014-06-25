#include "palette.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"

Bool Palette::CreateLayout(void)
{

	GePrint("Create Layout!");
    BaseContainer *wprefs=GetWorldContainerInstance();

    m_Settings=wprefs->GetContainer(PALETTE_ID);

    if (!GeDialog::CreateLayout()) return FALSE;

    SetTitle(GeLoadString(IDS_COLORPICKER));
	for(int i=0;i<9;i++){
		m_spotColors.Insert(PaletteColor(),i);
	}

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,1,0,String(),0);
		ScrollGroupBegin(1,BFH_SCALEFIT|BFV_SCALEFIT,SCROLLGROUP_HORIZ);
			GroupBegin(2,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
				for(int i=0;i<9;i++){
					C4DGadget *area = AddUserArea(20 + i,BFV_SCALEFIT);
					AttachUserArea(m_spotColors[i],area);
				}
			GroupEnd();
		GroupEnd();
    GroupEnd();

    return TRUE;
}

Palette::~Palette()
{
}

Bool Palette::InitValues(void)
{
    return TRUE;
}

Bool Palette::Command(LONG id,const BaseContainer &msg)
{
    switch (id)
    {
	default:
		break;
    }
    return GeDialog::Command(id,msg);
}

LONG Palette::Message(const BaseContainer& msg, BaseContainer& result)
{
    switch (msg.GetId())
    {
		case BFM_COLORCHOOSER_PARENTMESSAGE:
		{
			m_Settings.SetContainer(BFM_COLORCHOOSER_PARENTMESSAGE,msg);
			break;
		}
    }

    return GeDialog::Message(msg,result);
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