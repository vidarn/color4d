#include "colorpickerdialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"

// TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO
//
// Make sure icc profiles and transforms are correctly destroyed!
//
// TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO


static void SelectColorCallback(Color color, void *data)
{
	if(data != NULL){
		((ColorPickerDialog*)data)->UpdateColor(color);
	}
}

Bool ColorPickerDialog::CreateLayout(void)
{
    BaseContainer *wprefs=GetWorldContainerInstance();

    m_Settings=wprefs->GetContainer(COLORPICKER_ID);

    if (!GeDialog::CreateLayout()) return FALSE;

    SetTitle(GeLoadString(IDS_COLORPICKER));

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,1,0,String(),0);
		GroupBegin(1,BFH_SCALEFIT,3,1,String(),0);
			wheelArea = AddUserArea(IDC_COLORWHEEL,BFH_LEFT);	
			if (wheelArea) AttachUserArea(m_colorWheel,wheelArea);
			boxArea = AddUserArea(IDC_COLORBOX,BFH_LEFT);	
			if (boxArea) AttachUserArea(m_colorBox,boxArea);
			
			if(AddSubDialog(IDC_SLIDERS,BFH_SCALEFIT)){
				m_sliderSubDiag.SetColor(m_pColor);
				m_sliderSubDiag.SetParent(this);
				AttachSubDialog(&m_sliderSubDiag,IDC_SLIDERS);
			}

		GroupEnd();
		GroupBegin(7,BFH_SCALEFIT,1,0,String(),0);
			GroupBegin(8,BFH_SCALEFIT,0,1,String(),0);
				C4DGadget *area = AddUserArea(IDC_TEST4,BFH_SCALEFIT);
				AttachUserArea(m_previewColors[0],area);
				m_previewColors[0].SetParent(this);
			GroupEnd();
		GroupEnd();
		if(AddSubDialog(IDC_PALETTE,BFH_SCALEFIT)){
			AttachSubDialog(&m_paletteSubDiag,IDC_PALETTE);
			m_paletteSubDiag.SetSelectCallback(&SelectColorCallback,(void *)this);
			m_paletteSubDiag.SetDragable(FALSE);
		}
		AddDlgGroup(DLG_OK|DLG_CANCEL);
    GroupEnd();

	GeDynamicArray<Real> offsets;
	offsets.Insert( 0.0,0);
	m_colorWheel.SetOffsets(offsets);

    return TRUE;
}

ColorPickerDialog::~ColorPickerDialog()
{
}

Bool ColorPickerDialog::InitValues(void)
{
	UpdateColor(Color(*m_pColor).SetSource(COLOR_SOURCE_DISPLAY));
    if (!GeDialog::InitValues()) return FALSE;
    return TRUE;
}

Bool ColorPickerDialog::Command(LONG id,const BaseContainer &msg)
{
	LONG val;
	String str;
	Real rVal[4];
	Color col;
    switch (id)
    {
    case DLG_OK:
        {
			for(int i=0;i<3;i++){
				(*m_pColor)[i] = m_DisplayColor[i];
			}
            BaseContainer *wprefs=GetWorldContainerInstance();
            wprefs->SetContainer(COLORPICKER_ID,m_Settings);
            Close();
        }
        return TRUE;
    case DLG_CANCEL:
        Close();
        return TRUE;
    }
    return GeDialog::Command(id,msg);
}

Bool ColorPickerDialog::CoreMessage(LONG id,const BaseContainer &msg)
{
    return GeDialog::CoreMessage(id,msg);
}

LONG ColorPickerDialog::Message(const BaseContainer& msg, BaseContainer& result)
{
    switch (msg.GetId())
    {
		case BFM_COLORCHOOSER_PARENTMESSAGE:
			m_Settings.SetContainer(BFM_COLORCHOOSER_PARENTMESSAGE,msg);
			break;
    }

    return GeDialog::Message(msg,result);
}

void ColorPickerDialog::UpdateColor(Color color){
	m_DisplayColor = color.Convert(COLOR_SOURCE_DISPLAY);
	Color wheel = color.Convert(COLOR_SOURCE_WHEEL);
	m_colorWheel.UpdateColor(wheel);
	m_colorBox.UpdateColor(wheel);
	GeDynamicArray<Color> offsetColors;
	m_colorWheel.GetOffsetColors(offsetColors);
	for(LONG i=0;i<offsetColors.GetCount();i++){
		m_previewColors[i].UpdateColor(offsetColors[i].Convert(COLOR_SOURCE_DISPLAY));
	}
	m_sliderSubDiag.UpdateColorFromParent(color);
}