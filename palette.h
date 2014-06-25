#pragma once
#include "c4d.h"
#include "colorbox.h"
#include "colorwheel.h"
#include "colorslider.h"
#include "color.h"
#include "palettecolor.h"
#include "ge_dynamicarray.h"
#include "c4d_commanddata.h"

class Palette : public GeDialog
{
private:
    DescriptionCustomGui    *gad;
public:

	Palette():m_spotColors(NULL){}
	~Palette();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(LONG id,const BaseContainer &msg);
    virtual LONG Message(const BaseContainer& msg, BaseContainer& result);

	BaseContainer m_Settings;
    BasePlugin *m_pPlugin;

	GeDynamicArray<PaletteColor> m_spotColors;
	C4DGadget *spotArea;

};

class PaletteCommand : public CommandData
{
	private:
		Palette dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual LONG GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};