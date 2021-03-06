#pragma once
#include "c4d.h"
#include "colorbox.h"
#include "colorslider.h"
#include "color.h"
#include "colordialog.h"
#include "spotcolor.h"
#include "ge_dynamicarray.h"
#include "palettesubdialog.h"
#include "slidersubdialog.h"
#include "colorwheelsubdialog.h"

class ColorPickerDialog : public ColorDialog
{
private:
    DescriptionCustomGui    *gad;
public:

	ColorPickerDialog():m_wheelSubDiag(this),m_paletteSubDiag(1){ColorPickerDialog(new Vector(1.0f));}
    ColorPickerDialog(Vector *color):m_wheelSubDiag(this), m_pColor(color), m_spotColors(NULL),m_paletteSubDiag(1) {}
	~ColorPickerDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(Int32 id,const BaseContainer &msg);
    virtual Bool CoreMessage(Int32 id,const BaseContainer &msg);
    virtual Int32 Message(const BaseContainer& msg, BaseContainer& result);
    virtual Int32 GetSettingsID(void){return 8;}

	virtual void UpdateColor(Color color);
    virtual void UpdateWheel();

	void LoadSpotColors(Int32 index);

	BaseContainer m_Settings;
    BasePlugin *m_pPlugin;
    Vector *m_pColor;
    Color m_oldColor;

	Color m_DisplayColor;
	PaletteSubDialog m_paletteSubDiag;
	SliderSubDialog m_sliderSubDiag;
	ColorWheelSubDialog m_wheelSubDiag;
    
	SpotColor *m_spotColors;
	SpotColor m_previewColors[2];
	C4DGadget *spotArea;
};