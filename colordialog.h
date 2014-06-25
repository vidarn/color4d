#pragma once
#include "c4d.h"
#include "color.h"

class ColorDialog : public GeDialog
{
public:

	ColorDialog(){};
	~ColorDialog(){};

    virtual Bool CreateLayout(void) = 0;
    virtual Bool InitValues(void) = 0;
    virtual Bool Command(LONG id,const BaseContainer &msg) = 0;
    virtual Bool CoreMessage(LONG id,const BaseContainer &msg) = 0;
    virtual LONG Message(const BaseContainer& msg, BaseContainer& result) = 0;

	virtual void UpdateColor(Color color) = 0;
};