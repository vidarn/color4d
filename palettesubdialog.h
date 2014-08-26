#pragma once
#include "c4d.h"
#include "color.h"
#include "palettecolor.h"
#include "palette.h"
#include "ge_dynamicarray.h"
#include "c4d_commanddata.h"
#include "trashcan.h"

class PaletteSubDialog : public SubDialog
{
private:
    DescriptionCustomGui    *gad;
public:

	PaletteSubDialog(Int32 id);
	~PaletteSubDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(Int32 id,const BaseContainer &msg);
	virtual Bool CoreMessage(Int32 id, const BaseContainer& msg);

	void SetSelectCallback(void (*selectCallback)(Color, void *), void *data);
	void SetDragable(Bool state);
	void LoadPalette(Int32 id);
    void LayoutPalette();
	void PaletteLayout();
    void UpdatePopup(Int32 current);
    void ShowControls(Bool show);
    void ToContainer(BaseContainer &bc);
    void FromContainer(const BaseContainer &bc);
    void SaveSettings();

	BaseContainer m_Settings;
    
    enum Actions
    {
        ACTION_LOAD=FIRST_POPUP_ID,
        ACTION_NEW,
        ACTION_SAVE,
        ACTION_LABEL,
        ACTION_COUNT
    };

    Int32 m_id;
	Int32 m_paletteID;
    Int32 m_rows;
    Int32 m_layout;
    String m_searchString;
	Palette m_palette;
	PaletteColor *m_spotColors;
    TrashCan m_trash;
	C4DGadget *spotArea;
    C4DGadget *m_actionPopup;
    C4DGadget *m_trashArea;
    C4DGadget *m_rowArea;
    C4DGadget *m_layoutArea;
    C4DGadget *m_labelCheckArea;
    C4DGadget *m_nameArea;
    C4DGadget *m_searchText;
	void (*m_selectCallback)(Color, void *);
	void *m_selectCallbackData;
	Bool m_dragable;
    Bool m_showLabel;
    Bool m_showControls;
    Bool m_controlsShown;
};