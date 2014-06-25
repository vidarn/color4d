#include "colorselectordialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"

// TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO
//
// Make sure icc profiles and transforms are correctly destroyed!
//
// TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO


Bool ColorSelectorDialog::CreateLayout(void)
{
	m_displayProfile = cmsCreate_sRGBProfile();

	GePrint("Create Layout!");
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
		GroupEnd();
		GroupBegin(7,BFH_SCALEFIT,1,0,String(),0);
			GroupBegin(8,BFH_SCALEFIT,0,1,String(),0);
				for(int i=0;i<4;i++){
					C4DGadget *area = AddUserArea(IDC_TEST4+i,BFH_SCALEFIT);
					AttachUserArea(m_previewColors[i],area);
					m_previewColors[i].SetParent(this);
				}
			GroupEnd();
		GroupEnd();
    GroupEnd();

	GeDynamicArray<Real> offsets;
	offsets.Insert( 0.0,0);
	offsets.Insert( 0.1,1);
	offsets.Insert(-0.1,2);
	offsets.Insert( 0.5,3);
	m_colorWheel.SetOffsets(offsets);

    return TRUE;
}

ColorSelectorDialog::~ColorSelectorDialog()
{
}

Bool ColorSelectorDialog::InitValues(void)
{
    return TRUE;
}

Bool ColorSelectorDialog::Command(LONG id,const BaseContainer &msg)
{
	LONG val;
	Real rVal[4];
    switch (id)
    {
    }
    return GeDialog::Command(id,msg);
}

Bool ColorSelectorDialog::CoreMessage(LONG id,const BaseContainer &msg)
{
    return GeDialog::CoreMessage(id,msg);
}

LONG ColorSelectorDialog::Message(const BaseContainer& msg, BaseContainer& result)
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

void ColorSelectorDialog::UpdateColor(Color color){
	m_DisplayColor = color.Convert(COLOR_SOURCE_DISPLAY);
	Color wheel = color.Convert(COLOR_SOURCE_WHEEL);
	m_colorWheel.UpdateColor(wheel);
	m_colorBox.UpdateColor(wheel);
	GeDynamicArray<Color> offsetColors;
	m_colorWheel.GetOffsetColors(offsetColors);
	for(LONG i=0;i<offsetColors.GetCount();i++){
		m_previewColors[i].UpdateColor(offsetColors[i].Convert(COLOR_SOURCE_DISPLAY));
	}
}

LONG ColorSelectorCommand::GetState(BaseDocument *doc)
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