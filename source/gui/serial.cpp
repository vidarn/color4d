/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// Check for serial number

#include "c4d.h"
#include "c4d_symbols.h"

#define SERIAL_SIZE				 12
#define MYPLUGIN_ID		9999999 // CHANGE THIS !!!! 

Bool Validate(CHAR *nr)
{
	// example check for serial number
	if (nr[0]!='T') return FALSE;
	if (nr[1]!='E') return FALSE;
	if (nr[2]!='S') return FALSE;
	if (nr[3]!='T') return FALSE;
	if (nr[4]!='-') return FALSE;
	return nr[5]>='0' && nr[5]<='9';
}

class SerialDialog : public GeModalDialog
{
	private:
		CHAR *str;
	public:
		SerialDialog(CHAR *t_str);

		virtual Bool CreateLayout(void);
		virtual Bool InitValues(void);
		virtual Bool Command(LONG id, const BaseContainer &msg);
		virtual Bool AskClose(void);
};

SerialDialog::SerialDialog(CHAR *t_str)
{ 
	str = t_str; 
}

Bool SerialDialog::AskClose(void)
{
	String v;
	GetString(IDC_SERIALNUMBER,v);
	v.GetCString(str,SERIAL_SIZE-1);
	if (!Validate(str)) 
	{
		LONG time=GeGetTimer();
		GeShowMouse(MOUSE_BUSY);
		Enable(IDC_SERIALNUMBER,FALSE);
		while (GeGetTimer()-time<8000) {}
		Enable(IDC_SERIALNUMBER,TRUE);
		GeShowMouse(MOUSE_NORMAL);
		return TRUE;
	}
	return FALSE;
}

Bool SerialDialog::CreateLayout(void)
{
	return GeModalDialog::CreateLayout() && LoadDialogResource(DLG_REGISTER,NULL,0);
}

Bool SerialDialog::InitValues(void)
{
	// first call the parent instance
	if (!GeModalDialog::InitValues()) return FALSE;
	SetString(IDC_SERIALNUMBER,String(str));
	return TRUE;
}

Bool SerialDialog::Command(LONG id, const BaseContainer &msg)
{
	// read dialog values when user presses OK
	switch (id)
	{
		case IDC_OK: 
			String v;
			GetString(IDC_SERIALNUMBER,v);
			v.GetCString(str,SERIAL_SIZE-1);
			break;			
	}
	
	return TRUE;
}

Bool OpenSerialDialog(CHAR *sn)
{
	SerialDialog dlg(sn);
	return dlg.Open();
}

Bool CheckSerial(void) // call this routine AFTER resources have been initialized
{
	CHAR data[SERIAL_SIZE],sn[SERIAL_SIZE];

	if (!ReadPluginInfo(MYPLUGIN_ID,data,SERIAL_SIZE)) goto Error; 
	
	if (data[0]!='O' || data[1]!='K' || !Validate(data+4)) goto Error; // check your data here

	return TRUE;

Error:
	// serial number could not be authenticated
	ClearMem(sn,SERIAL_SIZE);
	
	if (!OpenSerialDialog(sn)) return FALSE;
	
	data[0]='O'; // write the serial number here
	data[1]='K'; 
	CopyMem(sn,data+4,SERIAL_SIZE-4);

	return WritePluginInfo(MYPLUGIN_ID,data,SERIAL_SIZE);
}

