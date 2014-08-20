#pragma once
#include "c4d.h"
#include "colorwheel.h"
#include "colorslider.h"

class ColorWheelSubDialog : public SubDialog
{
private:
    DescriptionCustomGui    *gad;
public:
    
	ColorWheelSubDialog(ColorDialog *parent);
	~ColorWheelSubDialog();
    
    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(Int32 id,const BaseContainer &msg);
    
	virtual void UpdateColor(Color color);
	virtual void UpdateColorFromParent(Color color);
	void SetColor(Vector *color){m_pColor = color;}
	void SetParent(ColorDialog *parent){m_parent = parent;}
    void SetWheelType(Int32 type);
    void ChangeWheelType(Int32 type);
    
	cmsHPROFILE m_displayProfile;
    ColorWheel m_colorWheel;
    ColorSlider m_colorSlider;
    
    BaseContainer *m_settings;
    Vector *m_pColor;
	ColorDialog *m_parent;
    C4DGadget *m_colorWheelArea;
    C4DGadget *m_colorSliderArea;
};