#pragma once
#include "c4d.h"
#include "colorbox.h"
#include "colorwheel.h"
#include "colorslider.h"
#include "color.h"
#include "palettecolor.h"
#include "palette.h"
#include "ge_dynamicarray.h"
#include "c4d_commanddata.h"

class PaletteDialog : public GeDialog
{
private:
    DescriptionCustomGui    *gad;
public:

	PaletteDialog();
	~PaletteDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(LONG id,const BaseContainer &msg);
    virtual LONG Message(const BaseContainer& msg, BaseContainer& result);
	virtual Bool CoreMessage(LONG id, const BaseContainer& msg);

	void LoadPalette(LONG id);
	void PaletteLayout();

	BaseContainer m_Settings;
    BasePlugin *m_pPlugin;

	LONG m_paletteID;
	Palette m_palette;
	PaletteColor *m_spotColors;
	C4DGadget *spotArea;

};

class PaletteCommand : public CommandData
{
	private:
		PaletteDialog dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual LONG GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};