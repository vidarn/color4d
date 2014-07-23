#pragma once
#include "c4d.h"
#include "colorbox.h"
#include "colorwheel.h"
#include "colorslider.h"
#include "color.h"
#include "spotcolor.h"
#include "ge_dynamicarray.h"
#include "c4d_commanddata.h"

class SpotColorDialog : public GeDialog
{
private:
    DescriptionCustomGui    *gad;
public:

	SpotColorDialog():m_spotColors(NULL), m_iccSearchPaths(NULL){}
	~SpotColorDialog();

    virtual Bool CreateLayout(void);
    virtual Bool InitValues(void);
    virtual Bool Command(LONG id,const BaseContainer &msg);
    virtual Bool CoreMessage(LONG id,const BaseContainer &msg);
    virtual LONG Message(const BaseContainer& msg, BaseContainer& result);

	void FindICCProfiles();
	void LoadSpotColors(LONG index);

	String *m_iccSearchPaths;

	cmsHPROFILE m_displayProfile;

	BaseContainer m_Settings;
    BasePlugin *m_pPlugin;

	GeDynamicArray<cmsHPROFILE> m_spotProfiles;

	Color m_DisplayColor;

	SpotColor *m_spotColors;
	C4DGadget *iccSpotCombo;
	C4DGadget *spotArea;
	C4DGadget *m_labelCheckbox;
	C4DGadget *m_searchText;

};

class SpotColorCommand : public CommandData
{
	private:
		SpotColorDialog dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual LONG GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};