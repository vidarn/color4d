#include "c4d_resource.h"
#include "lib_sn.h"
#include "c4d_symbols.h"

class ExampleSNHookClass : public SNHookClass
{
public:
	String name;

	ExampleSNHookClass()
	{
		name = GeLoadString(IDS_SERIAL_HOOK);
		if(!name.Content())
			name = "C++ SDK - Example Serial Hook";
	}

	virtual LONG SNCheck(const String &c4dsn,const String &sn,LONG regdate,LONG curdate)
	{
		return sn == String("123456789-abcdef") ? SN_OKAY : SN_WRONGNUMBER;
	}

	virtual const String& GetTitle()
	{
		return name;
	}
};

ExampleSNHookClass *snhook = NULL;

Bool RegisterExampleSNHook()
{
	snhook = gNew ExampleSNHookClass;
	if (!snhook->Register(450000241, SNFLAG_OWN))
		return FALSE;
	return TRUE;
}

void FreeExampleSNHook()
{
	if (snhook)
		gDelete(snhook);
}
