#include "palettedialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"
PaletteDialog::PaletteDialog()
{
}


Bool PaletteDialog::CreateLayout(void)
{
	GePrint("Create Layout!");
    BaseContainer *wprefs=GetWorldContainerInstance();
	LONG numColors = m_palette.GetCount();

    m_Settings=wprefs->GetContainer(PALETTE_ID);

    if (!GeDialog::CreateLayout()) return FALSE;

    SetTitle(GeLoadString(IDS_COLORPICKER));
	for(int i=0;i<numColors;i++){
		m_spotColors.Insert(PaletteColor(),i);
	}

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,1,0,String(),0);
		AddButton(1234,BFH_SCALEFIT,0,0,String("Button"));
		ScrollGroupBegin(1,BFH_SCALEFIT|BFV_SCALEFIT,SCROLLGROUP_HORIZ);
			GroupBegin(22,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
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
	Palette pal = Palette(String("Test"),3);
	pal.SetColor(0,Color(1.0f,0.f,0.f));
	pal.SetColor(1,Color(1.0f,1.f,0.f));
	pal.SetColor(2,Color(1.0f,1.f,1.f));
	LoadPalette(pal);
    return TRUE;
}

Bool PaletteDialog::Command(LONG id,const BaseContainer &msg)
{
	GeDynamicArray<Palette> pals;
    switch (id)
    {
		case 1234:
				Palette::GetPalettes(pals);
				LoadPalette(pals[0]);
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

void PaletteDialog::LoadPalette(const Palette &palette)
{
	m_palette = palette;
	
	LayoutFlushGroup(22);
	PaletteLayout();
	LayoutChanged(22);
}

void PaletteDialog::PaletteLayout()
{
	m_spotColors.SetCount(0);
	for(int i=0;i<m_palette.GetCount();i++){
		m_spotColors.Push(PaletteColor());
		C4DGadget *area = AddUserArea(1235 + i,BFV_SCALEFIT);
		AttachUserArea(m_spotColors[i],area);
		m_spotColors[i].SetColor(m_palette[i]);
		m_spotColors[i].SetColorID(i);
	}
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