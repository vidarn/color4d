#include "palettedialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"
PaletteDialog::PaletteDialog()
{
}


Bool PaletteDialog::CreateLayout(void)
{
<<<<<<< HEAD
	GePrint("Create Layout!");
    BaseContainer *wprefs=GetWorldContainerInstance();
	Int32 numColors = m_palette.GetCount();

    m_Settings=wprefs->GetContainer(PALETTE_ID);

=======
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
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

<<<<<<< HEAD
Bool PaletteDialog::InitValues(void)
{
	Palette pal = Palette(String("Test"),3);
	pal.SetColor(0,Color(1.0f,0.f,0.f));
	pal.SetColor(1,Color(1.0f,1.f,0.f));
	pal.SetColor(2,Color(1.0f,1.f,1.f));
	LoadPalette(pal);
    return TRUE;
}

Bool PaletteDialog::Command(Int32 id,const BaseContainer &msg)
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

Int32 PaletteDialog::Message(const BaseContainer& msg, BaseContainer& result)
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

Int32 PaletteCommand::GetState(BaseDocument *doc)
=======
LONG PaletteCommand::GetState(BaseDocument *doc)
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
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