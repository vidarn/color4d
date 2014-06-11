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
					iccSpotCombo = AddComboBox(IDC_SPOTICC,BFH_SCALEFIT,10,10);
				GroupEnd();
				GroupBegin(4,BFH_SCALEFIT,2,0,String(),0);
					for(LONG i=0;i<3;i++){
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
					for(LONG i=0;i<4;i++){
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
		ScrollGroupBegin(5,BFH_SCALEFIT|BFV_SCALEFIT,SCROLLGROUP_VERT);
			GroupBorderNoTitle(BORDER_THIN_IN);
			GroupBegin(6,BFH_SCALEFIT|BFV_SCALEFIT,7,0,String(),0);
			GroupEnd();
		GroupEnd();
		GroupBegin(7,BFH_SCALEFIT,1,0,String(),0);
			BaseContainer splineSettings;
			splineSettings.SetBool(SPLINECONTROL_ALLOW_HORIZ_SCALE_MOVE,true);
			splineSettings.SetBool(SPLINECONTROL_ALLOW_VERT_SCALE_MOVE,true);
			splineSettings.SetBool(SPLINECONTROL_GRID_H,true);
			splineSettings.SetBool(SPLINECONTROL_GRID_V,true);
			splineSettings.SetReal(SPLINECONTROL_X_MIN,0.0);
			splineSettings.SetReal(SPLINECONTROL_X_MAX,1.0);
			splineSettings.SetReal(SPLINECONTROL_X_STEPS,0.01);
			splineSettings.SetReal(SPLINECONTROL_Y_MIN,0.0);
			splineSettings.SetReal(SPLINECONTROL_Y_MAX,1.0);
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
				GeDynamicArray<Real> offsets;
				offsets.Insert( 0.0,0);
				offsets.Insert( 0.1,1);
				offsets.Insert(-0.1,2);
				offsets.Insert( 0.5,3);
				m_colorWheel.SetOffsets(offsets);
			}
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

        m.SetLong(BFM_COLORCHOOSER_SYSTEM   ,wprefs->GetLong(WPREF_COLOR_SYSTEM_BP));
        m.SetLong(BFM_COLORCHOOSER_RGB_RANGE,wprefs->GetLong(WPREF_COLOR_RGBRANGE));
        m.SetLong(BFM_COLORCHOOSER_H_RANGE  ,wprefs->GetLong(WPREF_COLOR_HRANGE));
        m.SetLong(BFM_COLORCHOOSER_SV_RANGE ,wprefs->GetLong(WPREF_COLOR_SVRANGE));
        m.SetLong(BFM_COLORCHOOSER_QUICKSTORE,wprefs->GetLong(WPREF_COLOR_QUICK_BP));
        m.SetLong(BFM_COLORCHOOSER_MIXINGPANEL,wprefs->GetLong(WPREF_COLOR_MIX_BP));

        m_Settings.SetContainer(BFM_COLORCHOOSER_PARENTMESSAGE,m);
        bc = m_Settings.GetContainerInstance(BFM_COLORCHOOSER_PARENTMESSAGE);
        if (!bc) return FALSE;
    }
    
    BaseContainer col(BFM_COLORCHOOSER);

    col.SetLong(BFM_COLORCHOOSER_SYSTEM   ,bc->GetLong(BFM_COLORCHOOSER_SYSTEM   ));
    col.SetLong(BFM_COLORCHOOSER_RGB_RANGE,bc->GetLong(BFM_COLORCHOOSER_RGB_RANGE));
    col.SetLong(BFM_COLORCHOOSER_H_RANGE  ,bc->GetLong(BFM_COLORCHOOSER_H_RANGE  ));
    col.SetLong(BFM_COLORCHOOSER_SV_RANGE ,bc->GetLong(BFM_COLORCHOOSER_SV_RANGE ));
    col.SetLong(BFM_COLORCHOOSER_QUICKSTORE,bc->GetLong(BFM_COLORCHOOSER_QUICKSTORE));
    col.SetLong(BFM_COLORCHOOSER_MIXINGPANEL,bc->GetLong(BFM_COLORCHOOSER_MIXINGPANEL));

    col.SetLong(BFM_COLORCHOOSER_SYSTEMMESSAGE,TRUE);

	FindICCProfiles();
	Color::SetWheelProfile(cmsCreate_sRGBProfile());
	Color::SetRGBProfile(cmsCreate_sRGBProfile());
	Color::SetCMYKProfile(m_CMYKProfiles[0]);
	Color::SetDisplayProfile(m_displayProfile);
	Color::UpdateTransforms();

    return TRUE;
}

ColorPickerDialog::~ColorPickerDialog()
{
	delete m_iccSearchPaths;
}

void ColorPickerDialog::FindICCProfiles(){
	m_iccSearchPaths = new String[1];
	m_iccSearchPaths[0] = "C:\\Windows\\System32\\Spool\\Drivers\\Color\\";

	BaseContainer RGBbc, CMYKbc, spotbc;

	cmsHPROFILE sRGBProfile;
	sRGBProfile = cmsCreate_sRGBProfile();
	m_RGBProfiles.Insert(sRGBProfile,0);;
	RGBbc.SetString(0,String("sRGB"));

	BrowseFiles* bf = BrowseFiles::Alloc();

	Filename dir(m_iccSearchPaths[0]);
	bf->Init(dir,FALSE);
	int RGBPos  = m_RGBProfiles.GetCount();
	int CMYKPos = m_CMYKProfiles.GetCount();
	int spotPos = m_spotProfiles.GetCount();

	if (bf)
	{
		while (bf->GetNext())
		{
			Filename fileName = bf->GetFilename();
			fileName.SetDirectory(dir);
			String str = fileName.GetString();
			CHAR *buffer = new CHAR[str.GetCStringLen()+1];
			str.GetCString(buffer,str.GetCStringLen()+1);
			cmsHPROFILE profile = cmsOpenProfileFromFile(buffer, "r");
			if(profile != NULL){
				cmsColorSpaceSignature sig = cmsGetColorSpace(profile);
				LONG length = cmsGetProfileInfoASCII(profile,cmsInfoDescription,"en","US",NULL,0);
				CHAR *buffer2 = new CHAR[length];
				cmsGetProfileInfoASCII(profile,cmsInfoDescription,"en","US",buffer2,length);
				String info(buffer2);
				int pt = _cmsLCMScolorSpace(sig);
				if(PT_RGB == pt){
					RGBbc.SetString(RGBPos,info);
					m_RGBProfiles.Insert(profile,RGBPos);
					RGBPos++;
				}
				if(PT_CMYK == pt){
					cmsHTRANSFORM xform = cmsCreateTransform(profile,TYPE_NAMED_COLOR_INDEX,m_displayProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
					if(xform != NULL){
						cmsNAMEDCOLORLIST* colorList = cmsGetNamedColorList(xform);
						if(colorList != NULL){
							spotbc.SetString(spotPos,info);
							m_spotProfiles.Insert(profile,spotPos);
							spotPos++;
						}
						else{
							CMYKbc.SetString(CMYKPos,info);
							m_CMYKProfiles.Insert(profile,CMYKPos);
							CMYKPos++;
						}
					}
				}
				delete buffer2;
			}
			delete buffer;
		}
		AddChildren(iccRGBCombo,RGBbc);
		SetLong(iccRGBCombo,0);
		Enable(iccRGBCombo,TRUE);

		AddChildren(iccCMYKCombo,CMYKbc);
		SetLong(iccCMYKCombo,0);
		Enable(iccCMYKCombo,TRUE);

		AddChildren(iccSpotCombo,spotbc);
		SetLong(iccSpotCombo,0);
		Enable(iccSpotCombo,TRUE);
	}
	
	BrowseFiles::Free(bf);
	ChangeRGBSliderProfile(0);
}

void ColorPickerDialog::ChangeRGBSliderProfile(LONG index)
{
	Color::SetRGBProfile(m_RGBProfiles[index],TRUE);
}

void ColorPickerDialog::ChangeCMYKSliderProfile(LONG index)
{
	Color::SetCMYKProfile(m_CMYKProfiles[index],TRUE);
}

void ColorPickerDialog::LoadSpotColors(LONG index)
{
	LayoutFlushGroup(6);
	double RGB[3];
	CHAR name[256], prefix[33], suffix[33];
	cmsHPROFILE profile = m_spotProfiles[index];
	cmsHTRANSFORM xform = cmsCreateTransform(profile,TYPE_NAMED_COLOR_INDEX,m_displayProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
	if(xform != NULL){
		cmsNAMEDCOLORLIST* colorList = cmsGetNamedColorList(xform);
		if(colorList != NULL){
			CHAR name[256], prefix[33], suffix[33];
			LONG numColors = cmsNamedColorCount(colorList);
			if(m_spotColors != NULL){
				delete m_spotColors;
			}
			m_spotColors = new SpotColor[numColors];
			for(int i=0;i<numColors;i+=7){
				LONG limit = i+7 < numColors ? i+7 : numColors;
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
	UpdateColor(Color(*m_pColor).SetSource(COLOR_SOURCE_DISPLAY));
    if (!GeDialog::InitValues()) return FALSE;
    return TRUE;
}

Bool ColorPickerDialog::Command(LONG id,const BaseContainer &msg)
{
	LONG val;
	Real rVal[4];
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
	case IDC_SPOTICC:
		val;
		GetLong(iccSpotCombo,val);
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

Bool ColorPickerDialog::CoreMessage(LONG id,const BaseContainer &msg)
{
    return GeDialog::CoreMessage(id,msg);
}

LONG ColorPickerDialog::Message(const BaseContainer& msg, BaseContainer& result)
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
	for(LONG i=0;i<3;i++){
		m_RGBSlider[i].UpdateColor(RGB);
		SetReal(RGBeditNumber[i],RGB[i],0.0,1.0,0.01,FORMAT_REAL);
	}
	Color CMYK = color.Convert(COLOR_SOURCE_CMYK);
	for(LONG i=0;i<4;i++){
		m_CMYKSlider[i].UpdateColor(CMYK);
		SetReal(CMYKeditNumber[i],CMYK[i]);
	}
	GeDynamicArray<Color> offsetColors;
	m_colorWheel.GetOffsetColors(offsetColors);
	for(LONG i=0;i<offsetColors.GetCount();i++){
		m_previewColors[i].UpdateColor(offsetColors[i].Convert(COLOR_SOURCE_DISPLAY));
	}
}