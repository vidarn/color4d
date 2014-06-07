#pragma once
#include "c4d.h"
#include "colorbox.h"
#include "colorwheel.h"
#include "colorslider.h"
#include "spotcolor.h"
#include "lcms2.h"
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

	void UpdateColor(cmsCIELab color);
	void FindICCProfiles();
	void ChangeSliderProfile(LONG index);
	void LoadSpotColors(LONG index);

	cmsCIELab WheelToLab(Vector color);
	Vector WheelTosRGB(const Vector &color);
	cmsCIELab RGBSlidersToLab(Vector color);
	Vector RGBSlidersTosRGB(const Vector &color);

	String *m_iccSearchPaths;
	
	cmsHTRANSFORM m_LabTosRGB;
	cmsHTRANSFORM m_sRGBToLab;
	cmsHTRANSFORM m_LabToWheel;
	cmsHTRANSFORM m_wheelToLab;
	cmsHTRANSFORM m_sRGBToWheel;
	cmsHTRANSFORM m_wheelTosRGB;
	cmsHTRANSFORM m_LabToRGBSliders;
	cmsHTRANSFORM m_RGBSlidersToLab;
	cmsHTRANSFORM m_sRGBToRGBSliders;
	cmsHTRANSFORM m_RGBSlidersTosRGB;

	cmsHPROFILE m_sRGBProfile, m_LabProfile;

	BaseContainer m_Settings;
    BasePlugin *m_pPlugin;
    Vector *m_pColor;

	GeDynamicArray<cmsHPROFILE> m_RGBProfiles;
	GeDynamicArray<cmsHPROFILE> m_CMYKProfiles;
	GeDynamicArray<cmsHPROFILE> m_spotProfiles;

	cmsCIELab m_color;
	ColorWheel m_colorWheel;
	ColorBox m_colorBox;
	ColorSlider m_RGBSlider[3];
	ColorSlider m_CMYKSlider[4];
	SpotColor *m_spotColors;
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