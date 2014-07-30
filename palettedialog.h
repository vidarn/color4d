#pragma once
#include "c4d.h"
#include "color.h"
#include "palettecolor.h"
#include "palette.h"
#include "ge_dynamicarray.h"
#include "c4d_commanddata.h"
#include "palettesubdialog.h"

class PaletteDialog : public GeDialog
{
private:
	PaletteSubDialog m_paletteSubDiag;
public:

	PaletteDialog();
	~PaletteDialog();

    virtual Bool CreateLayout(void);
<<<<<<< HEAD
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

=======
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
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