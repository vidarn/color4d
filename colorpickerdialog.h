#pragma once
#include "c4d.h"
#include "colorbox.h"
#include "colorwheel.h"
#include "colorslider.h"
#include "color.h"
#include "colordialog.h"
#include "spotcolor.h"
#include "ge_dynamicarray.h"
#include "palettesubdialog.h"
#include "slidersubdialog.h"

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
<<<<<<< HEAD
	void FindICCProfiles();
	void ChangeRGBSliderProfile(Int32 index);
	void ChangeCMYKSliderProfile(Int32 index);
	void LoadSpotColors(Int32 index);
=======
	void LoadSpotColors(LONG index);
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65

	BaseContainer m_Settings;
    BasePlugin *m_pPlugin;
    Vector *m_pColor;

	Color m_DisplayColor;
	PaletteSubDialog m_paletteSubDiag;
	SliderSubDialog m_sliderSubDiag;

	ColorWheel m_colorWheel;
	ColorBox m_colorBox;
	SpotColor *m_spotColors;
	SpotColor m_previewColors[4];
	C4DGadget *wheelArea;
	C4DGadget *boxArea;
	C4DGadget *spotArea;
};