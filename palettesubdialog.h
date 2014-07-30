#pragma once
#include "c4d.h"
#include "color.h"
#include "palettecolor.h"
#include "palette.h"
#include "ge_dynamicarray.h"
#include "c4d_commanddata.h"

class PaletteSubDialog : public SubDialog
{
private:
    DescriptionCustomGui    *gad;
public:

	PaletteSubDialog();
	~PaletteSubDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(Int32 id,const BaseContainer &msg);
	virtual Bool CoreMessage(Int32 id, const BaseContainer& msg);

	void SetSelectCallback(void (*selectCallback)(Color, void *), void *data);
	void SetDragable(Bool state);
	void LoadPalette(Int32 id);
	void PaletteLayout();

	BaseContainer m_Settings;

	Int32 m_paletteID;
	Palette m_palette;
	PaletteColor *m_spotColors;
	C4DGadget *spotArea;
	void (*m_selectCallback)(Color, void *);
	void *m_selectCallbackData;
	Bool m_dragable;
};