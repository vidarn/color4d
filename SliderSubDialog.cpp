#include "slidersubdialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "colorpickerdialog.h"

SliderSubDialog::SliderSubDialog()
{

}

SliderSubDialog::~SliderSubDialog()
{

}

Bool SliderSubDialog::CreateLayout(void)
{
    if (!GeDialog::CreateLayout()) return FALSE;

	GroupBegin(1,BFH_SCALEFIT,3,1,String(),0);
		GroupBegin(2,BFH_SCALEFIT,1,0,String("RGB"),BFV_BORDERGROUP_FOLD_OPEN);
			GroupBorder(BORDER_WITH_TITLE|BORDER_THIN_IN);
			GroupBegin(3,BFH_SCALEFIT,1,2,String(),0);
				AddStaticText(IDC_ICCLABEL,BFH_SCALEFIT,0,0,String("ICC Profile"),BORDER_NONE);
				iccRGBCombo  = AddComboBox(IDC_RGBICC,BFH_SCALEFIT,10,10);
			GroupEnd();
			GroupBegin(4,BFH_SCALEFIT,2,0,String(),0);
				for(LONG i=0;i<3;i++){
					RGBeditNumber[i] = AddEditNumberArrows(IDC_R+i,BFH_LEFT);
					RGBsliderArea[i] = AddUserArea(IDC_RSLIDER+i,BFH_SCALEFIT);	
					m_RGBSlider[i].SetParent(m_parent);
					m_RGBSlider[i].SetIndex(i);
					m_RGBSlider[i].SetColorSource(COLOR_SOURCE_RGB);
					if (RGBsliderArea[i]){
						AttachUserArea(m_RGBSlider[i],RGBsliderArea[i]);
					}
				}
			GroupEnd();
			m_hexText = AddEditText(IDC_HEXTEXT,BFH_SCALEFIT);
			iccCMYKCombo = AddComboBox(IDC_CMYKICC,BFH_SCALEFIT,10,10);
			GroupBegin(4,BFH_SCALEFIT,2,0,String(),0);
				for(LONG i=0;i<4;i++){
					CMYKeditNumber[i] = AddEditNumberArrows(IDC_C+i,BFH_LEFT);
					CMYKsliderArea[i] = AddUserArea(IDC_CSLIDER+i,BFH_SCALEFIT);	
					m_CMYKSlider[i].SetParent(m_parent);
					m_CMYKSlider[i].SetIndex(i);
					m_CMYKSlider[i].SetColorSource(COLOR_SOURCE_CMYK);
					m_CMYKSlider[i].SetValueMax(100.0);
					if (CMYKsliderArea[i]){
						AttachUserArea(m_CMYKSlider[i],CMYKsliderArea[i]);
					}
				}
			GroupEnd();
				
		GroupEnd();
	GroupEnd();
    return TRUE;
}

Bool SliderSubDialog::InitValues(void)
{
	FindICCProfiles();
	UpdateColor(Color(*m_pColor).SetSource(COLOR_SOURCE_DISPLAY));
    if (!GeDialog::InitValues()) return FALSE;
    return TRUE;
}

Bool SliderSubDialog::Command(LONG id,const BaseContainer &msg)
{
	LONG val;
	String str;
	Real rVal[4];
	Color col;
    switch (id)
    {
	case IDC_R:
	case IDC_G:
	case IDC_B:
		for(int i=0;i<3;i++){
			GetReal(RGBeditNumber[i],rVal[i]);
		}
		UpdateColor(Color(rVal[0],rVal[1],rVal[2]));
		break;
	case IDC_RGBICC:
		val;
		GetLong(iccRGBCombo,val);
		ChangeRGBSliderProfile(val);
		break;
	case IDC_CMYKICC:
		val;
		GetLong(iccCMYKCombo,val);
		ChangeCMYKSliderProfile(val);
		break;
	case IDC_HEXTEXT:
		GetString(m_hexText,str);
		col.SetSource(COLOR_SOURCE_RGB);
		if(col.FromString(str)){
			UpdateColor(col);
		}
		break;
    }
    return GeDialog::Command(id,msg);
}

void SliderSubDialog::FindICCProfiles(){
	const GeDynamicArray<vnColorProfile> &RGBProfiles = Color::getRGBProfiles();
	const GeDynamicArray<vnColorProfile> &CMYKProfiles = Color::getCMYKProfiles();
	const GeDynamicArray<vnColorProfile> &spotProfiles = Color::getSpotProfiles();

	BaseContainer RGBbc, CMYKbc, spotbc;

	for(int i=0;i<RGBProfiles.GetCount();i++){
		RGBbc.SetString(i,RGBProfiles[i].m_name);
	}
	for(int i=0;i<CMYKProfiles.GetCount();i++){
		CMYKbc.SetString(i,CMYKProfiles[i].m_name);
	}
	for(int i=0;i<spotProfiles.GetCount();i++){
		spotbc.SetString(i,spotProfiles[i].m_name);
	}
	AddChildren(iccRGBCombo,RGBbc);
	SetLong(iccRGBCombo,0);
	Enable(iccRGBCombo,TRUE);

	AddChildren(iccCMYKCombo,CMYKbc);
	SetLong(iccCMYKCombo,0);
	Enable(iccCMYKCombo,TRUE);

	ChangeRGBSliderProfile(0);
}

void SliderSubDialog::UpdateColor(Color color){
	m_parent->UpdateColor(color);
}

void SliderSubDialog::UpdateColorFromParent(Color color){
	Color wheel = color.Convert(COLOR_SOURCE_WHEEL);
	Color RGB = color.Convert(COLOR_SOURCE_RGB);
	for(LONG i=0;i<3;i++){
		m_RGBSlider[i].UpdateColor(RGB);
		SetReal(RGBeditNumber[i],RGB[i],0.0,1.0,0.01,FORMAT_REAL);
	}
	Color CMYK = color.Convert(COLOR_SOURCE_CMYK);
	for(LONG i=0;i<4;i++){
		m_CMYKSlider[i].UpdateColor(CMYK);
		SetReal(CMYKeditNumber[i],CMYK[i]);
	}
	String s;
	RGB.ToString(s);
	SetString(m_hexText,s);
}

void SliderSubDialog::ChangeRGBSliderProfile(LONG index)
{
	Color::SetRGBProfile(index,TRUE);
}

void SliderSubDialog::ChangeCMYKSliderProfile(LONG index)
{
	Color::SetCMYKProfile(index,TRUE);
}