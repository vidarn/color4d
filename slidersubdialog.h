#pragma once
#include "c4d.h"
#include "colorslider.h"
#include "color.h"
#include "ge_dynamicarray.h"

class SliderSubDialog : public SubDialog
{
private:
    DescriptionCustomGui    *gad;
public:

	SliderSubDialog();
	~SliderSubDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(Int32 id,const BaseContainer &msg);
    virtual Bool CoreMessage(Int32 id,const BaseContainer &msg);

	virtual void UpdateColor(Color color);
	virtual void UpdateColorFromParent(Color color);
	void ChangeRGBSliderProfile(Int32 index);
	void ChangeCMYKSliderProfile(Int32 index);
    void ChangeHSVProfile();
	void SetColor(Vector *color){m_pColor = color;}
	void SetParent(ColorDialog *parent){m_parent = parent;}
	void FindICCProfiles();

	cmsHPROFILE m_displayProfile;

    Vector *m_pColor;
	ColorDialog *m_parent;

	Color m_DisplayColor;

	ColorSlider m_RGBSlider[3];
	ColorSlider m_CMYKSlider[4];
    ColorSlider m_HSVSlider[3];
    ColorSlider m_LABSlider[3];
	C4DGadget *RGBeditNumber[3];
	C4DGadget *CMYKeditNumber[4];
    C4DGadget *HSVeditNumber[3];
    C4DGadget *LABeditNumber[3];
	C4DGadget *RGBsliderArea[3];
	C4DGadget *CMYKsliderArea[4];
    C4DGadget *HSVsliderArea[3];
    C4DGadget *LABsliderArea[3];
	C4DGadget *iccRGBCombo;
	C4DGadget *iccCMYKCombo;
    C4DGadget *iccHSVCombo;
	C4DGadget *m_hexText;
};