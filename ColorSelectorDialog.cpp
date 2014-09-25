#include "colorselectordialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"

Bool ColorSelectorDialog::CreateLayout(void)
{
    if (!GeDialog::CreateLayout()) return FALSE;

    SetTitle("Color scheme designer");
    
    m_color = Vector(0.5f,0.261f,0.375f);

    GroupBegin(10,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
        GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,1,0,String(),0);
            GroupBegin(1,BFH_SCALEFIT,3,1,String(),0);
            if(AddSubDialog(IDC_COLORWHEEL, BFH_SCALEFIT)){
                AttachSubDialog(&m_wheelSubDiag, IDC_COLORWHEEL);
            }
            GroupEnd();
            GroupBegin(8,BFH_SCALEFIT,0,1,String(),0);
            GroupEnd();
            m_schemeCombo = AddComboBox(IDC_SCHEMECOMBO,BFH_SCALEFIT,10,10);
        GroupEnd();
        if(AddSubDialog(IDC_SLIDERS,BFH_SCALEFIT)){
            m_sliderSubDiag.SetColor(&m_color);
            m_sliderSubDiag.SetParent(this);
            AttachSubDialog(&m_sliderSubDiag,IDC_SLIDERS);
        }
    GroupEnd();
    return TRUE;
}

ColorSelectorDialog::~ColorSelectorDialog()
{
}

Bool ColorSelectorDialog::InitValues(void)
{
	BaseContainer schemebc, HSVbc;
	Int32 numSchemes = ColorScheme::GetNumSchemes();
	for(Int32 i=0;i<numSchemes;i++){
		ColorScheme *scheme = ColorScheme::GetColorScheme(i);
		schemebc.SetString(i,scheme->GetName());
	}
	AddChildren(m_schemeCombo,schemebc);
	SetColorScheme(ColorScheme::GetColorScheme(0));
    UpdateColor(Color(0.5f,0.261f,0.375f).SetSource(COLOR_SOURCE_DISPLAY));
    return TRUE;
}

Bool ColorSelectorDialog::Command(Int32 id,const BaseContainer &msg)
{
	Int32 val;
	switch(id){
		case IDC_SCHEMECOMBO:
			GetInt32(m_schemeCombo,val);
			SetColorScheme(ColorScheme::GetColorScheme(val));
			break;
	}
    return GeDialog::Command(id,msg);
}

Bool ColorSelectorDialog::CoreMessage(Int32 id,const BaseContainer &msg)
{
    return GeDialog::CoreMessage(id,msg);
}

Int32 ColorSelectorDialog::Message(const BaseContainer& msg, BaseContainer& result)
{
    return GeDialog::Message(msg,result);
}

void ColorSelectorDialog::UpdateColor(Color color){
	Color wheel = color.Convert(COLOR_SOURCE_WHEEL);
	m_wheelSubDiag.UpdateColorFromParent(wheel);
	GeDynamicArray<Color> offsetColors;
	m_wheelSubDiag.m_colorWheel.GetOffsetColors(offsetColors);
	for(Int32 i=0;i<offsetColors.GetCount();i++){
		m_previewColors[i].UpdateColor(offsetColors[i].Convert(COLOR_SOURCE_DISPLAY));
	}
    m_sliderSubDiag.UpdateColorFromParent(color);
}

void ColorSelectorDialog::UpdateWheel()
{
    m_wheelSubDiag.m_colorWheel.UpdateCircle();
    m_wheelSubDiag.m_colorWheel.UpdateTriangle();
}

void ColorSelectorDialog::SetColorScheme(ColorScheme *colorScheme)
{
	m_colorScheme = colorScheme;
	m_wheelSubDiag.m_colorWheel.SetScheme(m_colorScheme);
	LayoutFlushGroup(8);
	for(int i=0;i<m_colorScheme->GetNumMarkers();i++){
		C4DGadget *area = AddUserArea(IDC_TEST4+i,BFH_SCALEFIT);
		AttachUserArea(m_previewColors[i],area);
		m_previewColors[i].SetParent(this);
	}
	LayoutChanged(8);
	UpdateColor(m_wheelSubDiag.m_colorWheel.GetColor());
}

Int32 ColorSelectorCommand::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}

Bool ColorSelectorCommand::Execute(BaseDocument *doc)
{
	return dlg.Open(DLG_TYPE_ASYNC,COLORSELECTOR_ID,-1,-1);
}

Bool ColorSelectorCommand::RestoreLayout(void *secret)
{
	return dlg.RestoreLayout(COLORSELECTOR_ID,0,secret);
}