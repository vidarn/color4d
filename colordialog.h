#pragma once
#include "c4d.h"
#include "color.h"

class ColorDialog : public GeDialog
{
public:

	ColorDialog(){};
	~ColorDialog(){};

    virtual Int32 GetSettingsID(void) = 0;
    virtual Bool CreateLayout(void) = 0;
    virtual Bool InitValues(void) = 0;
    virtual Bool Command(Int32 id,const BaseContainer &msg) = 0;
    virtual Bool CoreMessage(Int32 id,const BaseContainer &msg) = 0;
    virtual Int32 Message(const BaseContainer& msg, BaseContainer& result) = 0;

	virtual void UpdateColor(Color color) = 0;
};