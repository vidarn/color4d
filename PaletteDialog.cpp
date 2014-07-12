#include "palettedialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"
PaletteDialog::PaletteDialog()
{
}


Bool PaletteDialog::CreateLayout(void)
{
    BaseContainer *wprefs=GetWorldContainerInstance();

    m_Settings=wprefs->GetContainer(PALETTE_ID);

    if (!GeDialog::CreateLayout()) return FALSE;

    SetTitle(GeLoadString(IDS_COLORPICKER));

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,1,0,String(),0);
		AddButton(2,BFH_SCALEFIT,0,0,String("Button"));
		ScrollGroupBegin(1,BFH_SCALEFIT|BFV_SCALEFIT,SCROLLGROUP_HORIZ);
			GroupBegin(6,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
			GroupEnd();
		GroupEnd();
    GroupEnd();

    return TRUE;
}

PaletteDialog::~PaletteDialog()
{
}

Bool PaletteDialog::InitValues(void)
{
	LoadPalette(0);
    return TRUE;
}

Bool PaletteDialog::Command(LONG id,const BaseContainer &msg)
{
	GeDynamicArray<Palette> pals;
    switch (id)
    {
		case 2:
				Palette::GetPalettes(pals);
				LoadPalette(0);
				GePrint("Test!");
				break;
		default:
			break;
    }
    return GeDialog::Command(id,msg);
}

LONG PaletteDialog::Message(const BaseContainer& msg, BaseContainer& result)
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

Bool PaletteDialog::CoreMessage(LONG id, const BaseContainer& msg)
{
	if(GeIsMainThread()){
		GePrint("Main thread");
	}else{
		GePrint("Not main thread");
	}
    switch ( id )
    {
      case  PALETTE_ID:                                      // internal message
			LONG color =  (LONG) msg.GetVoid( BFM_CORE_PAR1 );
			LONG palette = (LONG) msg.GetVoid( BFM_CORE_PAR2 );
			if(palette == m_paletteID && color == -1){
				GePrint("Update palette!");
				LoadPalette(m_paletteID);
			}
        break;
    }
    return GeDialog::CoreMessage( id, msg );
}

void PaletteDialog::LoadPalette(LONG id)
{
	GeDynamicArray<Palette> pals;
	Palette::GetPalettes(pals);
	m_palette = pals[id];
	m_paletteID = id;
	
	PaletteLayout();

}

void PaletteDialog::PaletteLayout()
{
	LayoutFlushGroup(6);
	m_spotColors = new PaletteColor[m_palette.GetCount()];
	GePrint("PaletteLayout: " + LongToString(m_palette.GetCount()));
	for(int i=0;i<m_palette.GetCount();i++){
		m_spotColors[i].SetColor(m_palette[i]);
		m_spotColors[i].SetColorID(i);
		C4DGadget *area = AddUserArea(12 + i,BFV_SCALEFIT);
		AttachUserArea(m_spotColors[i],area);
	}
	LayoutChanged(6);
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