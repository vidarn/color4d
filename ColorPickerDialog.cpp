#include "colorpickerdialog.h"
#include "main.h"
#include "c4d_symbols.h"

Bool ColorPickerDialog::CreateLayout(void)
{
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
			GroupBegin(2,BFH_SCALEFIT|BFV_SCALEFIT,2,3,String(),0);
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

    return TRUE;
}

Bool ColorPickerDialog::InitValues(void)
{
	UpdateColor(RGBToHSV(*m_pColor));
	for(LONG i=0;i<3;i++){
		SetReal(editNumber[i],(*m_pColor)[i],0.0,1.0,0.01,FORMAT_REAL);
	}

    if (!GeDialog::InitValues()) return FALSE;
    return TRUE;
}

Bool ColorPickerDialog::Command(LONG id,const BaseContainer &msg)
{
    switch (id)
    {
	case IDC_R:
		m_color.x = msg.GetReal(BFM_ACTION_VALUE);
		UpdateColor(m_color);
		break;
	case IDC_G:
		m_color.y = msg.GetReal(BFM_ACTION_VALUE);
		UpdateColor(m_color);
		break;
	case IDC_B:
		m_color.z = msg.GetReal(BFM_ACTION_VALUE);
		UpdateColor(m_color);
		break;
    case DLG_OK:
        {
            Real b;
			*m_pColor = HSLtoRGB(m_colorWheel.GetColor());
			GePrint(RealToString(m_pColor->x) + " " + RealToString(m_pColor->y) + " " + RealToString(m_pColor->z));
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

void ColorPickerDialog::UpdateColor(Vector color){
	m_color = color;
	m_colorWheel.UpdateColor(color);
	m_colorBox.UpdateColor(color);
	m_redSlider.UpdateColor(color);
	m_greenSlider.UpdateColor(color);
	m_blueSlider.UpdateColor(color);
	for(LONG i=0;i<3;i++){
		SetReal(editNumber[i],color[i],0.0,1.0,0.01,FORMAT_REAL);
	}
}