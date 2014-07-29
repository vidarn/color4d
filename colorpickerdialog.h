#pragma once
#include "c4d.h"
#include "colorbox.h"
#include "colorwheel.h"
#include "colorslider.h"
#include "color.h"
#include "colordialog.h"
#include "spotcolor.h"
#include "ge_dynamicarray.h"

class ColorPickerDialog : public ColorDialog
{
private:
    DescriptionCustomGui    *gad;
public:

	ColorPickerDialog():m_colorWheel(this), m_colorBox(this){ColorPickerDialog(new Vector(1.0f));}
    ColorPickerDialog(Vector *color):m_colorWheel(this), m_colorBox(this), m_pColor(color), m_spotColors(NULL) {}
	~ColorPickerDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(Int32 id,const BaseContainer &msg);
    virtual Bool CoreMessage(Int32 id,const BaseContainer &msg);
    virtual Int32 Message(const BaseContainer& msg, BaseContainer& result);

	virtual void UpdateColor(Color color);
	void FindICCProfiles();
	void ChangeRGBSliderProfile(Int32 index);
	void ChangeCMYKSliderProfile(Int32 index);
	void LoadSpotColors(Int32 index);

	cmsHPROFILE m_displayProfile;

	BaseContainer m_Settings;
    BasePlugin *m_pPlugin;
    Vector *m_pColor;

	//GeDynamicArray<cmsHPROFILE> &m_RGBProfiles;
	//GeDynamicArray<cmsHPROFILE> &m_CMYKProfiles;
	//GeDynamicArray<cmsHPROFILE> &m_spotProfiles;

	Color m_DisplayColor;

	ColorWheel m_colorWheel;
	ColorBox m_colorBox;
	ColorSlider m_RGBSlider[3];
	ColorSlider m_CMYKSlider[4];
	SpotColor *m_spotColors;
	SpotColor m_previewColors[4];
	C4DGadget *RGBeditNumber[3];
	C4DGadget *CMYKeditNumber[4];
	C4DGadget *wheelArea;
	C4DGadget *boxArea;
	C4DGadget *RGBsliderArea[3];
	C4DGadget *CMYKsliderArea[4];
	C4DGadget *iccRGBCombo;
	C4DGadget *iccCMYKCombo;
	C4DGadget *iccSpotCombo;
	C4DGadget *spotArea;

};