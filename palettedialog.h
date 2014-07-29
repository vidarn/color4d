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
    virtual Bool Command(Int32 id,const BaseContainer &msg);
    virtual Int32 Message(const BaseContainer& msg, BaseContainer& result);

	void LoadPalette(const Palette &palette);
	void PaletteLayout();

	BaseContainer m_Settings;
    BasePlugin *m_pPlugin;

	Palette m_palette;
	GeDynamicArray<PaletteColor> m_spotColors;
	C4DGadget *spotArea;

};

class PaletteCommand : public CommandData
{
	private:
		PaletteDialog dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual Int32 GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};