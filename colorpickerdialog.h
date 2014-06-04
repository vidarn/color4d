#pragma once
#include "c4d.h"
#include "colorbox.h"
#include "colorwheel.h"
#include "colorslider.h"

class ColorPickerDialog : public GeDialog
{
private:
    DescriptionCustomGui    *gad;
public:

    ColorPickerDialog(Vector *color):m_colorWheel(this), m_colorBox(this), m_redSlider(this,0), m_greenSlider(this,1), m_blueSlider(this,2), m_pColor(color) {}

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(LONG id,const BaseContainer &msg);
    virtual Bool CoreMessage(LONG id,const BaseContainer &msg);
    virtual LONG Message(const BaseContainer& msg, BaseContainer& result);

	void UpdateColor(Vector color);

    BaseContainer m_Settings;
    BasePlugin *m_pPlugin;
    Vector *m_pColor;

	Vector m_color;
	ColorWheel m_colorWheel;
	ColorBox m_colorBox;
	ColorSlider m_redSlider;
	ColorSlider m_greenSlider;
	ColorSlider m_blueSlider;
	C4DGadget *editNumber[3];
	C4DGadget *wheelArea;
	C4DGadget *boxArea;
	C4DGadget *sliderArea[3];

};