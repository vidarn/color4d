#pragma once
#include "colordialog.h"
#include "colorbox.h"
#include "colorwheel.h"
#include "colorslider.h"
#include "color.h"
#include "spotcolor.h"
#include "ge_dynamicarray.h"

class ColorSelectorDialog : public ColorDialog
{
private:
    DescriptionCustomGui    *gad;
public:

	ColorSelectorDialog():m_colorWheel(this), m_colorBox(this){ColorSelectorDialog(new Vector(1.0f));}
    ColorSelectorDialog(Vector *color):m_colorWheel(this), m_colorBox(this){}
	~ColorSelectorDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(LONG id,const BaseContainer &msg);
    virtual Bool CoreMessage(LONG id,const BaseContainer &msg);
	virtual LONG Message(const BaseContainer& msg, BaseContainer& result);

	virtual void UpdateColor(Color color);

	void SetColorScheme(ColorScheme *colorScheme);

	ColorWheel m_colorWheel;
	ColorBox m_colorBox;
	SpotColor m_previewColors[4];
	ColorScheme *m_colorScheme;
	C4DGadget *wheelArea;
	C4DGadget *boxArea;
	C4DGadget *spotArea;
	C4DGadget *m_schemeCombo;
};

class ColorSelectorCommand : public CommandData
{
	private:
		ColorSelectorDialog dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual LONG GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};