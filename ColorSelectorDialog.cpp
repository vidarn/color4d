#include "colorselectordialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"

Bool ColorSelectorDialog::CreateLayout(void)
{
    if (!GeDialog::CreateLayout()) return FALSE;

    SetTitle(GeLoadString(IDS_COLORPICKER));

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,1,0,String(),0);
		GroupBegin(1,BFH_SCALEFIT,3,1,String(),0);
			wheelArea = AddUserArea(IDC_COLORWHEEL,BFH_LEFT);	
			if (wheelArea) AttachUserArea(m_colorWheel,wheelArea);
			boxArea = AddUserArea(IDC_COLORBOX,BFH_LEFT);	
			if (boxArea) AttachUserArea(m_colorBox,boxArea);
		GroupEnd();
		GroupBegin(8,BFH_SCALEFIT,0,1,String(),0);
		GroupEnd();
		m_schemeCombo = AddComboBox(IDC_SCHEMECOMBO,BFH_SCALEFIT,10,10);
    GroupEnd();

<<<<<<< HEAD
	GeDynamicArray<Float> offsets;
	offsets.Insert( 0.0,0);
	offsets.Insert( 0.1,1);
	offsets.Insert(-0.1,2);
	offsets.Insert( 0.5,3);
	m_colorWheel.SetOffsets(offsets);

=======
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
    return TRUE;
}

ColorSelectorDialog::~ColorSelectorDialog()
{
}

Bool ColorSelectorDialog::InitValues(void)
{
	BaseContainer schemebc;
	LONG numSchemes = ColorScheme::GetNumSchemes();
	for(LONG i=0;i<numSchemes;i++){
		ColorScheme *scheme = ColorScheme::GetColorScheme(i);
		schemebc.SetString(i,scheme->GetName());
	}
	AddChildren(m_schemeCombo,schemebc);
	SetColorScheme(ColorScheme::GetColorScheme(0));
    return TRUE;
}

Bool ColorSelectorDialog::Command(Int32 id,const BaseContainer &msg)
{
<<<<<<< HEAD
	Int32 val;
	Float rVal[4];
    switch (id)
    {
    }
=======
	LONG val;
	switch(id){
		case IDC_SCHEMECOMBO:
			GetLong(m_schemeCombo,val);
			SetColorScheme(ColorScheme::GetColorScheme(val));
			break;
	}
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
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
	m_colorWheel.UpdateColor(wheel);
	m_colorBox.UpdateColor(wheel);
	GeDynamicArray<Color> offsetColors;
	m_colorWheel.GetOffsetColors(offsetColors);
	for(Int32 i=0;i<offsetColors.GetCount();i++){
		m_previewColors[i].UpdateColor(offsetColors[i].Convert(COLOR_SOURCE_DISPLAY));
	}
}

<<<<<<< HEAD
Int32 ColorSelectorCommand::GetState(BaseDocument *doc)
=======
void ColorSelectorDialog::SetColorScheme(ColorScheme *colorScheme)
{
	m_colorScheme = colorScheme;
	m_colorWheel.SetScheme(m_colorScheme);
	LayoutFlushGroup(8);
	for(int i=0;i<m_colorScheme->GetNumMarkers();i++){
		C4DGadget *area = AddUserArea(IDC_TEST4+i,BFH_SCALEFIT);
		AttachUserArea(m_previewColors[i],area);
		m_previewColors[i].SetParent(this);
	}
	LayoutChanged(8);
	UpdateColor(m_colorWheel.GetColor());
}

LONG ColorSelectorCommand::GetState(BaseDocument *doc)
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
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