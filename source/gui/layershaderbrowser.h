// LayerShaderBrowser.h
//////////////////////////////////////////////////////////////////////

#ifndef _LAYERSHADERBROWSER_H_
#define _LAYERSHADERBROWSER_H_

#include "c4d_gui.h"

class LayerShaderBrowser : public GeDialog
{
public:
	LayerShaderBrowser();
	virtual ~LayerShaderBrowser();

	virtual Bool CreateLayout(void);
	virtual Bool InitValues(void);
	virtual Bool Command(LONG id, const BaseContainer &msg);
	virtual LONG Message(const BaseContainer &msg, BaseContainer &result);
	virtual Bool CoreMessage  (LONG id,const BaseContainer &msg);

	void UpdateAll(Bool msg);
	void ShowInfo(LayerShaderLayer* l);

	void *lastselected;

private:
	LinkBoxGui* linkbox;
	TreeViewCustomGui* tree;
	LONG lastdirty;
};

#endif // _LAYERSHADERBROWSER_H_
