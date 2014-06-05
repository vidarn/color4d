#pragma once
#include "c4d.h"
#include "colorbox.h"
#include "colorwheel.h"
#include "colorslider.h"
#include "lcms2.h"
#include "ge_dynamicarray.h"

class ColorPickerDialog : public GeDialog
{
private:
    DescriptionCustomGui    *gad;
public:

    ColorPickerDialog(Vector *color):m_colorWheel(this), m_colorBox(this), m_redSlider(this,0), m_greenSlider(this,1), m_blueSlider(this,2), m_pColor(color) {}
	~ColorPickerDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(LONG id,const BaseContainer &msg);
    virtual Bool CoreMessage(LONG id,const BaseContainer &msg);
    virtual LONG Message(const BaseContainer& msg, BaseContainer& result);

	void UpdateColor(cmsCIELab color);
	void FindICCProfiles();
	void ChangeSliderProfile(LONG index);

	cmsCIELab WheelToLab(Vector color);
	Vector WheelTosRGB(const Vector &color);
	cmsCIELab SlidersToLab(Vector color);
	Vector SlidersTosRGB(const Vector &color);

	String *m_iccSearchPaths;
	
	cmsHTRANSFORM m_LabTosRGB;
	cmsHTRANSFORM m_sRGBToLab;
	cmsHTRANSFORM m_LabToWheel;
	cmsHTRANSFORM m_wheelToLab;
	cmsHTRANSFORM m_sRGBToWheel;
	cmsHTRANSFORM m_wheelTosRGB;
	cmsHTRANSFORM m_LabToSliders;
	cmsHTRANSFORM m_slidersToLab;
	cmsHTRANSFORM m_sRGBToSliders;
	cmsHTRANSFORM m_slidersTosRGB;

	cmsHPROFILE m_sRGBProfile, m_LabProfile;

	BaseContainer m_Settings;
    BasePlugin *m_pPlugin;
    Vector *m_pColor;

	GeDynamicArray<cmsHPROFILE> m_profiles;

	cmsCIELab m_color;
	ColorWheel m_colorWheel;
	ColorBox m_colorBox;
	ColorSlider m_redSlider;
	ColorSlider m_greenSlider;
	ColorSlider m_blueSlider;
	C4DGadget *editNumber[3];
	C4DGadget *wheelArea;
	C4DGadget *boxArea;
	C4DGadget *sliderArea[3];
	C4DGadget *iccCombo;

};