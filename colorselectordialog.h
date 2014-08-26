#pragma once
#include "colordialog.h"
#include "colorbox.h"
#include "colorwheelsubdialog.h"
#include "slidersubdialog.h"
#include "colorslider.h"
#include "color.h"
#include "spotcolor.h"
#include "ge_dynamicarray.h"

class ColorSelectorDialog : public ColorDialog
{
private:
    DescriptionCustomGui    *gad;
public:

	ColorSelectorDialog():m_wheelSubDiag(this){ColorSelectorDialog(new Vector(1.0f));}
    ColorSelectorDialog(Vector *color):m_wheelSubDiag(this){}
	~ColorSelectorDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(Int32 id,const BaseContainer &msg);
    virtual Bool CoreMessage(Int32 id,const BaseContainer &msg);
    virtual Int32 Message(const BaseContainer& msg, BaseContainer& result);
    virtual Int32 GetSettingsID(void){return 9;}

	virtual void UpdateColor(Color color);
    virtual void UpdateWheel();

	void SetColorScheme(ColorScheme *colorScheme);

    ColorWheelSubDialog m_wheelSubDiag;
    SliderSubDialog m_sliderSubDiag;
	SpotColor m_previewColors[4];
	ColorScheme *m_colorScheme;
	C4DGadget *boxArea;
	C4DGadget *spotArea;
	C4DGadget *m_schemeCombo;
    Vector m_color;
};

class ColorSelectorCommand : public CommandData
{
	private:
		ColorSelectorDialog dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual Int32 GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};