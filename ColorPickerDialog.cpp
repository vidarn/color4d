#include "colorpickerdialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"

// TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO
//
// Make sure icc profiles and transforms are correctly destroyed!
//
// TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO


Bool ColorPickerDialog::CreateLayout(void)
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
			GroupBegin(2,BFH_SCALEFIT,1,0,String("RGB"),BFV_BORDERGROUP_FOLD_OPEN);
				GroupBorder(BORDER_WITH_TITLE|BORDER_THIN_IN);
				GroupBegin(3,BFH_SCALEFIT,1,2,String(),0);
					AddStaticText(IDC_ICCLABEL,BFH_SCALEFIT,0,0,String("ICC Profile"),BORDER_NONE);
					iccRGBCombo  = AddComboBox(IDC_RGBICC,BFH_SCALEFIT,10,10);
					//iccSpotCombo = AddComboBox(IDC_SPOTICC,BFH_SCALEFIT,10,10);
				GroupEnd();
				GroupBegin(4,BFH_SCALEFIT,2,0,String(),0);
					for(Int32 i=0;i<3;i++){
						RGBeditNumber[i] = AddEditNumberArrows(IDC_R+i,BFH_LEFT);
						RGBsliderArea[i] = AddUserArea(IDC_RSLIDER+i,BFH_SCALEFIT);	
						m_RGBSlider[i].SetParent(this);
						m_RGBSlider[i].SetIndex(i);
						m_RGBSlider[i].SetColorSource(COLOR_SOURCE_RGB);
						if (RGBsliderArea[i]){
							AttachUserArea(m_RGBSlider[i],RGBsliderArea[i]);
						}
					}
				GroupEnd();
				iccCMYKCombo = AddComboBox(IDC_CMYKICC,BFH_SCALEFIT,10,10);
				GroupBegin(4,BFH_SCALEFIT,2,0,String(),0);
					for(Int32 i=0;i<4;i++){
						CMYKeditNumber[i] = AddEditNumberArrows(IDC_C+i,BFH_LEFT);
						CMYKsliderArea[i] = AddUserArea(IDC_CSLIDER+i,BFH_SCALEFIT);	
						m_CMYKSlider[i].SetParent(this);
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
		/*ScrollGroupBegin(5,BFH_SCALEFIT|BFV_SCALEFIT,SCROLLGROUP_VERT);
			GroupBorderNoTitle(BORDER_THIN_IN);
			GroupBegin(6,BFH_SCALEFIT|BFV_SCALEFIT,7,0,String(),0);
			GroupEnd();
		GroupEnd();*/
		GroupBegin(7,BFH_SCALEFIT,1,0,String(),0);
			/*BaseContainer splineSettings;
			splineSettings.SetBool(SPLINECONTROL_ALLOW_HORIZ_SCALE_MOVE,true);
			splineSettings.SetBool(SPLINECONTROL_ALLOW_VERT_SCALE_MOVE,true);
			splineSettings.SetBool(SPLINECONTROL_GRID_H,true);
			splineSettings.SetBool(SPLINECONTROL_GRID_V,true);
			splineSettings.SetFloat(SPLINECONTROL_X_MIN,0.0);
			splineSettings.SetFloat(SPLINECONTROL_X_MAX,1.0);
			splineSettings.SetFloat(SPLINECONTROL_X_STEPS,0.01);
			splineSettings.SetFloat(SPLINECONTROL_Y_MIN,0.0);
			splineSettings.SetFloat(SPLINECONTROL_Y_MAX,1.0);
			splineSettings.SetBool(SPLINECONTROL_NO_FLOATING_WINDOW,true);
			splineSettings.SetBool(SPLINECONTROL_VALUE_EDIT_H,true);
			splineSettings.SetBool(SPLINECONTROL_VALUE_EDIT_V,true);
			SplineCustomGui* pGui = (SplineCustomGui*)AddCustomGui(IDC_TEST2,CUSTOMGUI_SPLINE,String("Test"),BFH_SCALEFIT,100,100,splineSettings);
			if(pGui != NULL){
				SplineData *data = SplineData::Alloc();
				data->InsertKnot(0.0,0.0,FLAG_KNOT_LOCK_X|FLAG_KNOT_LOCK_Y);
				data->InsertKnot(1.0,1.0,NULL);
				pGui->SetSpline(data);
				SplineData::Free(data);
			}*/
			GroupBegin(8,BFH_SCALEFIT,0,1,String(),0);
				for(int i=0;i<4;i++){
					C4DGadget *area = AddUserArea(IDC_TEST4+i,BFH_SCALEFIT);
					AttachUserArea(m_previewColors[i],area);
					m_previewColors[i].SetParent(this);
				}
			GroupEnd();
		GroupEnd();
		AddDlgGroup(DLG_OK|DLG_CANCEL);
    GroupEnd();
    
    BaseContainer *bc=m_Settings.GetContainerInstance(BFM_COLORCHOOSER_PARENTMESSAGE);
    if (!bc)
    {
        BaseContainer m;

        m.SetInt32(BFM_COLORCHOOSER_SYSTEM   ,wprefs->GetInt32(WPREF_COLOR_SYSTEM_BP));
        m.SetInt32(BFM_COLORCHOOSER_RGB_RANGE,wprefs->GetInt32(WPREF_COLOR_RGBRANGE));
        m.SetInt32(BFM_COLORCHOOSER_H_RANGE  ,wprefs->GetInt32(WPREF_COLOR_HRANGE));
        m.SetInt32(BFM_COLORCHOOSER_SV_RANGE ,wprefs->GetInt32(WPREF_COLOR_SVRANGE));
        m.SetInt32(BFM_COLORCHOOSER_QUICKSTORE,wprefs->GetInt32(WPREF_COLOR_QUICK_BP));
        m.SetInt32(BFM_COLORCHOOSER_MIXINGPANEL,wprefs->GetInt32(WPREF_COLOR_MIX_BP));

        m_Settings.SetContainer(BFM_COLORCHOOSER_PARENTMESSAGE,m);
        bc = m_Settings.GetContainerInstance(BFM_COLORCHOOSER_PARENTMESSAGE);
        if (!bc) return FALSE;
    }
    
    BaseContainer col(BFM_COLORCHOOSER);

    col.SetInt32(BFM_COLORCHOOSER_SYSTEM   ,bc->GetInt32(BFM_COLORCHOOSER_SYSTEM   ));
    col.SetInt32(BFM_COLORCHOOSER_RGB_RANGE,bc->GetInt32(BFM_COLORCHOOSER_RGB_RANGE));
    col.SetInt32(BFM_COLORCHOOSER_H_RANGE  ,bc->GetInt32(BFM_COLORCHOOSER_H_RANGE  ));
    col.SetInt32(BFM_COLORCHOOSER_SV_RANGE ,bc->GetInt32(BFM_COLORCHOOSER_SV_RANGE ));
    col.SetInt32(BFM_COLORCHOOSER_QUICKSTORE,bc->GetInt32(BFM_COLORCHOOSER_QUICKSTORE));
    col.SetInt32(BFM_COLORCHOOSER_MIXINGPANEL,bc->GetInt32(BFM_COLORCHOOSER_MIXINGPANEL));

    col.SetInt32(BFM_COLORCHOOSER_SYSTEMMESSAGE,TRUE);

	GeDynamicArray<Float> offsets;
	offsets.Insert( 0.0,0);
	offsets.Insert( 0.1,1);
	offsets.Insert(-0.1,2);
	offsets.Insert( 0.5,3);
	m_colorWheel.SetOffsets(offsets);

	FindICCProfiles();

    return TRUE;
}

ColorPickerDialog::~ColorPickerDialog()
{
}

void ColorPickerDialog::FindICCProfiles(){
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
	SetInt32(iccRGBCombo,0);
	Enable(iccRGBCombo,TRUE);

	AddChildren(iccCMYKCombo,CMYKbc);
	SetInt32(iccCMYKCombo,0);
	Enable(iccCMYKCombo,TRUE);

	ChangeRGBSliderProfile(0);
}

void ColorPickerDialog::ChangeRGBSliderProfile(Int32 index)
{
	Color::SetRGBProfile(index,TRUE);
}

void ColorPickerDialog::ChangeCMYKSliderProfile(Int32 index)
{
	Color::SetCMYKProfile(index,TRUE);
}

void ColorPickerDialog::LoadSpotColors(Int32 index)
{
	LayoutFlushGroup(6);
	double RGB[3];
	Char name[256], prefix[33], suffix[33];
	cmsHPROFILE profile = Color::getSpotProfiles()[index].m_profile;
	cmsHTRANSFORM xform = cmsCreateTransform(profile,TYPE_NAMED_COLOR_INDEX,m_displayProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
	if(xform != NULL){
		cmsNAMEDCOLORLIST* colorList = cmsGetNamedColorList(xform);
		if(colorList != NULL){
			Char name[256], prefix[33], suffix[33];
			Int32 numColors = cmsNamedColorCount(colorList);
			if(m_spotColors != NULL){
				delete m_spotColors;
			}
			m_spotColors = new SpotColor[numColors];
			for(int i=0;i<numColors;i+=7){
				Int32 limit = i+7 < numColors ? i+7 : numColors;
				for(int ii=i;ii<limit;ii++){
					cmsNamedColorInfo(colorList,ii,name,prefix,suffix,NULL,NULL);
					Color col;
					cmsDoTransform(xform,&ii,RGB,1);
					for(int a=0;a<3;a++){
						col[a] = RGB[a];
					}
					col.SetSource(COLOR_SOURCE_DISPLAY);
					m_spotColors[ii].SetParent(this);

					GroupBegin(ii+IDC_LASTENTRY,BFH_SCALEFIT,1,0,String(name)+String(suffix),FALSE);
					GroupBorder(BORDER_WITH_TITLE|BORDER_THIN_IN);
					C4DGadget *area = AddUserArea(ii*2+IDC_LASTENTRY,BFH_SCALEFIT);
					AttachUserArea(m_spotColors[ii],area);
					m_spotColors[ii].UpdateColor(col);

					GroupEnd();
				}
			}
		}
	}
	else{
		GePrint("NULL!");
	}
	LayoutChanged(6);
}

Bool ColorPickerDialog::InitValues(void)
{
	/*
	UpdateColor(Color(*m_pColor).SetSource(COLOR_SOURCE_DISPLAY));
    if (!GeDialog::InitValues()) return FALSE;*/
    return TRUE;
}

Bool ColorPickerDialog::Command(Int32 id,const BaseContainer &msg)
{
	Int32 val;
	Float rVal[4];
    switch (id)
    {
	case IDC_R:
	case IDC_G:
	case IDC_B:
		for(int i=0;i<3;i++){
			GetFloat(RGBeditNumber[i],rVal[i]);
		}
		UpdateColor(Color(rVal[0],rVal[1],rVal[2]));
		break;
	case IDC_RGBICC:
		val;
		GetInt32(iccRGBCombo,val);
		ChangeRGBSliderProfile(val);
		break;
	case IDC_CMYKICC:
		val;
		GetInt32(iccCMYKCombo,val);
		ChangeCMYKSliderProfile(val);
		break;
	case IDC_SPOTICC:
		val;
		GetInt32(iccSpotCombo,val);
		LoadSpotColors(val);
		break;
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

Bool ColorPickerDialog::CoreMessage(Int32 id,const BaseContainer &msg)
{
    return GeDialog::CoreMessage(id,msg);
}

Int32 ColorPickerDialog::Message(const BaseContainer& msg, BaseContainer& result)
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

void ColorPickerDialog::UpdateColor(Color color){
	m_DisplayColor = color.Convert(COLOR_SOURCE_DISPLAY);
	Color wheel = color.Convert(COLOR_SOURCE_WHEEL);
	m_colorWheel.UpdateColor(wheel);
	m_colorBox.UpdateColor(wheel);
	Color RGB = color.Convert(COLOR_SOURCE_RGB);
	for(Int32 i=0;i<3;i++){
		m_RGBSlider[i].UpdateColor(RGB);
		SetFloat(RGBeditNumber[i],RGB[i],0.0,1.0,0.01,FORMAT_FLOAT);
	}
	Color CMYK = color.Convert(COLOR_SOURCE_CMYK);
	for(Int32 i=0;i<4;i++){
		m_CMYKSlider[i].UpdateColor(CMYK);
		SetFloat(CMYKeditNumber[i],CMYK[i]);
	}
	GeDynamicArray<Color> offsetColors;
	m_colorWheel.GetOffsetColors(offsetColors);
	for(Int32 i=0;i<offsetColors.GetCount();i++){
		m_previewColors[i].UpdateColor(offsetColors[i].Convert(COLOR_SOURCE_DISPLAY));
	}
}