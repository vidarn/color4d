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
	GePrint("Create Layout!");
    BaseContainer *wprefs=GetWorldContainerInstance();

    m_Settings=wprefs->GetContainer(COLORPICKER_ID);

    if (!GeDialog::CreateLayout()) return FALSE;

    SetTitle(GeLoadString(IDS_COLORPICKER));

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,1,3,String(),0);
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
			GePrint(str);
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
					//TODO: Fix this!
					cmsHPROFILE bla = cmsCreate_sRGBProfile();
					cmsHTRANSFORM xform = cmsCreateTransform(profile,TYPE_NAMED_COLOR_INDEX,bla,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
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
			else{
				GePrint("Could not open!");
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
	ChangeSliderProfile(0);
}

void ColorPickerDialog::ChangeSliderProfile(LONG index)
{
	m_sRGBProfile = cmsCreate_sRGBProfile();
	m_LabProfile = cmsCreateLab4Profile(cmsD50_xyY());
	cmsHPROFILE profile = m_RGBProfiles[index];

	m_RGBSlidersToLab = cmsCreateTransform(profile,TYPE_RGB_DBL,m_LabProfile,TYPE_Lab_DBL,INTENT_PERCEPTUAL,0);
	m_LabToRGBSliders = cmsCreateTransform(m_LabProfile,TYPE_Lab_DBL,profile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);

	m_RGBSlidersTosRGB = cmsCreateTransform(profile,TYPE_RGB_DBL,m_sRGBProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
	m_sRGBToRGBSliders = cmsCreateTransform(m_sRGBProfile,TYPE_RGB_DBL,profile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
}

void ColorPickerDialog::LoadSpotColors(LONG index)
{
	LayoutFlushGroup(6);
	double RGB[3];
	CHAR name[256], prefix[33], suffix[33];
	cmsHPROFILE profile = m_spotProfiles[index];
	cmsHTRANSFORM xform = cmsCreateTransform(profile,TYPE_NAMED_COLOR_INDEX,m_sRGBProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
	if(xform != NULL){
		GePrint("bra bra");
		cmsNAMEDCOLORLIST* colorList = cmsGetNamedColorList(xform);
		if(colorList != NULL){
			GePrint("hejsan!");
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
					Vector col;
					cmsDoTransform(xform,&ii,RGB,1);
					col.x = RGB[0]; col.y = RGB[1]; col.z = RGB[2];
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
	cmsHPROFILE wheelProfile;
	wheelProfile = cmsCreate_sRGBProfile();

	m_sRGBToLab = cmsCreateTransform(m_sRGBProfile,TYPE_RGB_DBL,m_LabProfile,TYPE_Lab_DBL,INTENT_PERCEPTUAL,0);
	m_LabTosRGB = cmsCreateTransform(m_LabProfile,TYPE_Lab_DBL,m_sRGBProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);

	m_wheelToLab = cmsCreateTransform(wheelProfile,TYPE_RGB_DBL,m_LabProfile,TYPE_Lab_DBL,INTENT_PERCEPTUAL,0);
	m_LabToWheel = cmsCreateTransform(m_LabProfile,TYPE_Lab_DBL,wheelProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);

	m_wheelTosRGB = cmsCreateTransform(wheelProfile,TYPE_RGB_DBL,m_sRGBProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
	m_sRGBToWheel = cmsCreateTransform(m_sRGBProfile,TYPE_RGB_DBL,wheelProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);

	double RGB[3];
	for(int i=0;i<3;i++){
		RGB[i] = (*m_pColor)[i];
		GePrint(RealToString(RGB[i]));
	}

	cmsDoTransform(m_sRGBToLab,RGB,&m_color,1);

	UpdateColor(m_color);

    if (!GeDialog::InitValues()) return FALSE;
    return TRUE;
}

Bool ColorPickerDialog::Command(LONG id,const BaseContainer &msg)
{
	LONG val;
    switch (id)
    {
	case IDC_R:
		m_color.L = msg.GetReal(BFM_ACTION_VALUE);
		UpdateColor(m_color);
		break;
	case IDC_G:
		m_color.a = msg.GetReal(BFM_ACTION_VALUE);
		UpdateColor(m_color);
		break;
	case IDC_B:
		m_color.b = msg.GetReal(BFM_ACTION_VALUE);
		UpdateColor(m_color);
		break;
	case IDC_RGBICC:
		val;
		GetLong(iccRGBCombo,val);
		ChangeSliderProfile(val);
		break;
	case IDC_SPOTICC:
		val;
		GetLong(iccSpotCombo,val);
		LoadSpotColors(val);
		break;
    case DLG_OK:
        {
			double RGB[3];
			cmsDoTransform(m_LabTosRGB,&m_color,RGB,1);
			for(int i=0;i<3;i++){
				(*m_pColor)[i] = RGB[i];
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

void ColorPickerDialog::UpdateColor(cmsCIELab color){
	m_color = color;
	double RGB[3];
	cmsDoTransform(m_LabToWheel,&m_color,RGB,1);

	Vector col(RGB[0],RGB[1],RGB[2]);
	col = RGBToHSL(col);
	m_colorWheel.UpdateColor(col);
	m_colorBox.UpdateColor(col);

	cmsDoTransform(m_LabToRGBSliders,&m_color,RGB,1);
	col = Vector(RGB[0],RGB[1],RGB[2]);

	for(LONG i=0;i<3;i++){
		m_RGBSlider[i].UpdateColor(col);
		SetReal(RGBeditNumber[i],col[i],0.0,1.0,0.01,FORMAT_REAL);
	}
}

cmsCIELab ColorPickerDialog::WheelToLab(Vector color){
	cmsCIELab lab;
	double RGB[3];
	color = HSLtoRGB(color);
	RGB[0] = color.x; RGB[1] = color.y; RGB[2] = color.z;
	cmsDoTransform(m_wheelToLab,RGB,&lab,1);
	return lab;
}

Vector ColorPickerDialog::WheelTosRGB(const Vector &color){
	return HSLtoRGB(color);
}

cmsCIELab ColorPickerDialog::RGBSlidersToLab(Vector color){
	cmsCIELab lab;
	double RGB[3];
	RGB[0] = color.x; RGB[1] = color.y; RGB[2] = color.z;
	cmsDoTransform(m_RGBSlidersToLab,RGB,&lab,1);
	return lab;
}

Vector ColorPickerDialog::RGBSlidersTosRGB(const Vector &color){
	double in[3], out[3];
	in[0] = color.x; in[1] = color.y; in[2] = color.z;
	cmsDoTransform(m_RGBSlidersTosRGB,in,out,1);
	Vector ret = Vector(out[0], out[1],out[2]); 
	return ret;
}