#pragma once
#include "c4d.h"
#include "colorbox.h"
#include "colorwheel.h"
#include "colorslider.h"
#include "color.h"
#include "spotcolor.h"
#include "ge_dynamicarray.h"

class ColorPickerDialog : public GeDialog
{
private:
    DescriptionCustomGui    *gad;
public:

    ColorPickerDialog(Vector *color):m_colorWheel(this), m_colorBox(this), m_pColor(color), m_spotColors(NULL) {}
	~ColorPickerDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(LONG id,const BaseContainer &msg);
    virtual Bool CoreMessage(LONG id,const BaseContainer &msg);
    virtual LONG Message(const BaseContainer& msg, BaseContainer& result);

	void UpdateColor(Color color);
	void FindICCProfiles();
	void ChangeRGBSliderProfile(LONG index);
	void ChangeCMYKSliderProfile(LONG index);
	void LoadSpotColors(LONG index);

	String *m_iccSearchPaths;

	cmsHPROFILE m_displayProfile;

	BaseContainer m_Settings;
    BasePlugin *m_pPlugin;
    Vector *m_pColor;

	GeDynamicArray<cmsHPROFILE> m_RGBProfiles;
	GeDynamicArray<cmsHPROFILE> m_CMYKProfiles;
	GeDynamicArray<cmsHPROFILE> m_spotProfiles;

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