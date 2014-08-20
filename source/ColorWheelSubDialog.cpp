#include "colorwheelsubdialog.h"
#include "colordialog.h"

ColorWheelSubDialog::ColorWheelSubDialog(ColorDialog *parent):m_colorWheel(parent,this),m_settings(NULL),m_parent(parent)
{
}

ColorWheelSubDialog::~ColorWheelSubDialog()
{
    
}

Bool ColorWheelSubDialog::CreateLayout(void)
{
    if (!GeDialog::CreateLayout()) return FALSE;
    
	GroupBegin(1,BFH_SCALEFIT,1,0,String(),0);
        m_colorWheelArea = AddUserArea(IDC_COLORWHEEL,BFH_SCALEFIT);
        if (m_colorWheelArea){
            AttachUserArea(m_colorWheel,m_colorWheelArea);
        }
        GroupBegin(2, BFH_SCALEFIT,1,0,String(),0);
        GroupEnd();
	GroupEnd();
    return TRUE;
}

Bool ColorWheelSubDialog::InitValues(void)
{
    if (!GeDialog::InitValues()) return FALSE;
    return TRUE;
}

Bool ColorWheelSubDialog::Command(Int32 id,const BaseContainer &msg)
{
    return GeDialog::Command(id,msg);
}

void ColorWheelSubDialog::UpdateColor(Color color)
{
	m_parent->UpdateColor(color);
}

void ColorWheelSubDialog::UpdateColorFromParent(Color color)
{
    m_colorWheel.UpdateColor(color);
    m_colorSlider.UpdateColor(color);
}

void ColorWheelSubDialog::SetWheelType(Int32 type)
{
    if(m_settings != NULL){
        GePrint(String::IntToString(m_settings->GetInt32(m_parent->GetSettingsID(), type)));
        m_settings->SetInt32(m_parent->GetSettingsID(), type);
    }
    LayoutFlushGroup(2);
    if(type == COLOR_WHEEL_BLENDER){
        m_colorSliderArea = AddUserArea(IDC_SLIDERS,BFH_SCALEFIT|BFV_TOP);
        if (m_colorSliderArea){
            AttachUserArea(m_colorSlider,m_colorSliderArea);
            m_colorSlider.SetParent(m_colorWheel.m_parent);
            m_colorSlider.SetIndex(2);
        }
    }
    LayoutChanged(2);
}

void ColorWheelSubDialog::ChangeWheelType(Int32 type)
{
    m_colorWheel.m_type = type;
    m_colorWheel.UpdateCircle();
    m_colorWheel.UpdateTriangle();
    SetWheelType(type);
    m_colorWheel.Redraw();
}
