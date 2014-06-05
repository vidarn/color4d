#include "colorpickerdialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"

Bool ColorPickerDialog::CreateLayout(void)
{
	GePrint("Create Layout!");
    BaseContainer *wprefs=GetWorldContainerInstance();

    m_Settings=wprefs->GetContainer(COLORPICKER_ID);

    if (!GeDialog::CreateLayout()) return FALSE;

    SetTitle(GeLoadString(IDS_COLORPICKER));

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,1,2,String(),0);
		GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,3,1,String(),0);
			wheelArea = AddUserArea(IDC_COLORWHEEL,BFH_LEFT);	
			if (wheelArea) AttachUserArea(m_colorWheel,wheelArea);
			boxArea = AddUserArea(IDC_COLORBOX,BFH_LEFT);	
			if (boxArea) AttachUserArea(m_colorBox,boxArea);
			GroupBegin(2,BFH_SCALEFIT|BFV_SCALEFIT,1,2,String(),0);
				GroupBegin(3,BFH_SCALEFIT|BFV_SCALEFIT,1,2,String(),0);
					AddStaticText(IDC_ICCLABEL,BFH_SCALEFIT,0,0,String("ICC Profile"),BORDER_NONE);
					iccCombo = AddComboBox(IDC_ICC,BFH_SCALEFIT,10,10);
				GroupEnd();
				GroupBegin(4,BFH_SCALEFIT|BFV_SCALEFIT,2,3,String(),0);
					editNumber[0] = AddEditNumberArrows(IDC_R,BFH_LEFT);
					sliderArea[0] = AddUserArea(IDC_RSLIDER,BFH_LEFT);	
					if (sliderArea[0]) AttachUserArea(m_redSlider,sliderArea[0]);

					editNumber[1] = AddEditNumberArrows(IDC_G,BFH_LEFT);
					sliderArea[1] = AddUserArea(IDC_GSLIDER,BFH_LEFT);	
					if (sliderArea[1]) AttachUserArea(m_greenSlider,sliderArea[1]);

					editNumber[2] = AddEditNumberArrows(IDC_B,BFH_LEFT);
					sliderArea[2] = AddUserArea(IDC_BSLIDER,BFH_LEFT);	
					if (sliderArea[2]) AttachUserArea(m_blueSlider,sliderArea[2]);
				GroupEnd();
			GroupEnd();
		GroupEnd();
		AddDlgGroup(DLG_OK|DLG_CANCEL);
    GroupEnd();

    //////////////////////////////////////////////////////////////////////////
    
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

	BaseContainer bc;

	cmsHPROFILE sRGBProfile;
	sRGBProfile = cmsCreate_sRGBProfile();
	m_profiles.Insert(sRGBProfile,0);;
	bc.SetString(0,String("sRGB"));

	BrowseFiles* bf = BrowseFiles::Alloc();

	GePrint("haj haj!");

	Filename dir(m_iccSearchPaths[0]);
	bf->Init(dir,FALSE);
	int i = m_profiles.GetCount();
	if (bf)
	{
		GePrint("hsdgsdg!");
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
				LONG length = cmsGetProfileInfoASCII(profile,cmsInfoDescription,"en","US",NULL,0);
				GePrint(LongToString(length));
				CHAR *buffer2 = new CHAR[length];
				cmsGetProfileInfoASCII(profile,cmsInfoDescription,"en","US",buffer2,length);
				String info(buffer2);
				GePrint(info);
				bc.SetString(i,info);
				m_profiles.Insert(profile,i);
				i++;
				delete buffer2;
			}
			else{
				GePrint("Could not open!");
			}
			delete buffer;
		}
		AddChildren(iccCombo,bc);
		SetLong(iccCombo,0);
		Enable(iccCombo,TRUE);
	}
	
	BrowseFiles::Free(bf);
	ChangeSliderProfile(0);
}

void ColorPickerDialog::ChangeSliderProfile(LONG index)
{
	m_sRGBProfile = cmsCreate_sRGBProfile();
	m_LabProfile = cmsCreateLab4Profile(cmsD50_xyY());
	cmsHPROFILE profile = m_profiles[index];
	cmsColorSpaceSignature sig = cmsGetColorSpace(profile);
	cmsUInt32Number numChannels = cmsChannelsOf(sig);
	int pt = _cmsLCMScolorSpace(sig);
	if(PT_RGB == pt){
		GePrint(String("RGB!"));
	}
	if(PT_CMYK == pt){
		GePrint(String("CMYK!"));
	}

	m_slidersToLab = cmsCreateTransform(profile,TYPE_RGB_DBL,m_LabProfile,TYPE_Lab_DBL,INTENT_PERCEPTUAL,0);
	m_LabToSliders = cmsCreateTransform(m_LabProfile,TYPE_Lab_DBL,profile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);

	m_slidersTosRGB = cmsCreateTransform(profile,TYPE_RGB_DBL,m_sRGBProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
	m_sRGBToSliders = cmsCreateTransform(m_sRGBProfile,TYPE_RGB_DBL,profile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
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
	case IDC_ICC:
		LONG val;
		GetLong(iccCombo,val);
		ChangeSliderProfile(val);
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

	cmsDoTransform(m_LabToSliders,&m_color,RGB,1);
	col = Vector(RGB[0],RGB[1],RGB[2]);

	m_redSlider.UpdateColor(col);
	m_greenSlider.UpdateColor(col);
	m_blueSlider.UpdateColor(col);
	for(LONG i=0;i<3;i++){
		SetReal(editNumber[i],col[i],0.0,1.0,0.01,FORMAT_REAL);
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

cmsCIELab ColorPickerDialog::SlidersToLab(Vector color){
	cmsCIELab lab;
	double RGB[3];
	RGB[0] = color.x; RGB[1] = color.y; RGB[2] = color.z;
	cmsDoTransform(m_slidersToLab,RGB,&lab,1);
	return lab;
}

Vector ColorPickerDialog::SlidersTosRGB(const Vector &color){
	double in[3], out[3];
	in[0] = color.x; in[1] = color.y; in[2] = color.z;
	cmsDoTransform(m_slidersTosRGB,in,out,1);
	Vector ret = Vector(out[0], out[1],out[2]); 
	return ret;
}