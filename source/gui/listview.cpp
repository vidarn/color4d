/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code for usage of listview elements

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_LISTVIEWTEST 1000452

#include "c4d.h"
#include "c4d_symbols.h"

struct TestData
{
	LONG id;
	CHAR name[20];
};

TestData testdata[] =
{
	{ 10,"Sharpen" },
	{ 12,"LensFlares" },
	{ 11,"Emboss" },
	{ 14,"Depth of Field" },
	{ 214,"AA" },

	{ 0,"" }
};

class ListViewDialog : public GeDialog
{
	private:
		SimpleListView				listview1;
		SimpleListView				listview2;
		AutoAlloc<BaseSelect>	selection;
		LONG									counter2;

		void UpdateButtons(void);

	public:
		ListViewDialog(void);
		virtual ~ListViewDialog(void);

		virtual Bool CreateLayout(void);
		virtual Bool InitValues(void);
		virtual Bool Command(LONG id,const BaseContainer &msg);
		virtual LONG Message(const BaseContainer &msg,BaseContainer &result);

};

enum
{
	IDC_OFFSET = 1001,
	IDC_ACCESS = 1002
};

ListViewDialog::ListViewDialog(void)
{
	counter2  = 0;
}

ListViewDialog::~ListViewDialog(void)
{
}

Bool ListViewDialog::CreateLayout(void)
{
	// first call the parent instance
	Bool res = GeDialog::CreateLayout();

	res = LoadDialogResource(DLG_LISTVIEW,NULL,0);

	if (res)
	{
		listview1.AttachListView(this,GADGET_LISTVIEW1);
		listview2.AttachListView(this,GADGET_LISTVIEW2);
	}

	return res;
}

void ListViewDialog::UpdateButtons(void)
{
	if (!selection) return;

	Enable(GADGET_INSERT,listview1.GetSelection(selection));
	Enable(GADGET_REMOVE,listview2.GetSelection(selection));
}

Bool ListViewDialog::InitValues(void)
{
	// first call the parent instance
	if (!GeDialog::InitValues()) return FALSE;

	BaseContainer layout;
	BaseContainer data;
	LONG i=0;

	layout.SetLong('name',LV_COLUMN_TEXT);
//	layout.SetLong('used',LV_COLUMN_CHECKBOX);
	listview1.SetLayout(1,layout);

	layout = BaseContainer();
	layout.SetLong('chck',LV_COLUMN_CHECKBOX);
	layout.SetLong('name',LV_COLUMN_TEXT);
	layout.SetLong('bttn',LV_COLUMN_BUTTON);
	listview2.SetLayout(3,layout);

	data = BaseContainer();

	for (i=0;testdata[i].id;i++)
	{
		data.SetString('name',testdata[i].name);
		//data.SetLong('used',FALSE);
		listview1.SetItem(testdata[i].id,data);
	}

	//data = BaseContainer();
	//for (i=0;testdata[i].id;i++)
	//{
	//	data.SetLong('chck',TRUE);
	//	data.SetString('name',testdata[i].name);
	//	data.SetString('bttn',"...");
	//	listview2.SetItem(testdata[i].id,data);
	//}

	listview1.DataChanged();
	listview2.DataChanged();

	UpdateButtons();

	return TRUE;
}

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void GePrintF(const CHAR *format,...)
{
	va_list arp;
	CHAR buf[1024];

	va_start(arp,format);
	vsprintf_safe(buf,sizeof(buf),format,arp);
	GePrint(buf);
	va_end(arp);
}

Bool ListViewDialog::Command(LONG id,const BaseContainer &msg)
{
	switch (id)
	{
		case GADGET_LISTVIEW1:
		case GADGET_LISTVIEW2:
			{
				switch (msg.GetLong(BFM_ACTION_VALUE))
				{
					case LV_SIMPLE_SELECTIONCHANGED:
						GePrintF("Selection changed, id: %d, val: %p ",msg.GetLong(LV_SIMPLE_ITEM_ID),msg.GetVoid(LV_SIMPLE_DATA));
						break;

					case LV_SIMPLE_CHECKBOXCHANGED:
						GePrintF("CheckBox changed, id: %d, col: %d, val: %p",msg.GetLong(LV_SIMPLE_ITEM_ID),msg.GetLong(LV_SIMPLE_COL_ID),msg.GetVoid(LV_SIMPLE_DATA));
						break;

					case LV_SIMPLE_FOCUSITEM:
						GePrintF("Focus set id: %d, col: %d",msg.GetLong(LV_SIMPLE_ITEM_ID),msg.GetLong(LV_SIMPLE_COL_ID));
						break;

					case LV_SIMPLE_BUTTONCLICK:
						GePrintF("Button clicked id: %d, col: %d",msg.GetLong(LV_SIMPLE_ITEM_ID),msg.GetLong(LV_SIMPLE_COL_ID));
						break;
				}
			}
			UpdateButtons();
			break;

//		case GADGET_LISTVIEW2:
//			break;

		case GADGET_INSERT:
			{
				AutoAlloc<BaseSelect> s2;
				if (selection && s2)
				{
					// TEST
					LONG i,id,count = listview1.GetItemCount();
					BaseContainer test;

					for (i=0;i<count;i++)
					{
						listview1.GetItemLine(i,&id,&test);
					}
					// TEST

					if (!listview1.GetSelection(selection))
					{
						GePrint("No Selection");
					}
					else
					{
						LONG i,a,b;
						String str;
						for (i=0;selection->GetRange(i,MAXLONGl,&a,&b);i++)
						{
							if (a==b) str += LongToString(a)+" ";
							else str += LongToString(a)+"-"+LongToString(b)+" ";
						}
		//				str.Delete(str.GetLength()-1,1);
						GePrint("Selection: "+str);

						BaseContainer data;
						for (i=0;testdata[i].id;i++)
						{
							if (selection->IsSelected(testdata[i].id))
							{
								data.SetLong('chck',TRUE);
								data.SetString('name',testdata[i].name);
								data.SetString('bttn',"...");
								selection->Select(counter2);
								listview2.SetItem(counter2++,data);
							}
						}
						listview2.SetSelection(selection);
						listview2.DataChanged();
					}
				}
			}
			UpdateButtons();
			break;

		case GADGET_REMOVE:
			{
				if (selection && listview2.GetSelection(selection))
				{
					LONG i,a,b;
					for (i=0;selection->GetRange(i,MAXLONGl,&a,&b);i++)
					{
						for (;a<=b;a++)
						{
							listview2.RemoveItem(a);
						}
					}
					listview2.DataChanged();
				}
			}
			UpdateButtons();
			break;
	}
	return TRUE;
}


LONG ListViewDialog::Message(const BaseContainer &msg,BaseContainer &result)
{
//	switch (msg.GetId())
	{
	}
	return GeDialog::Message(msg,result);
}

class ListViewTest : public CommandData
{
	private:
		ListViewDialog dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual LONG GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};

LONG ListViewTest::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}

Bool ListViewTest::Execute(BaseDocument *doc)
{
	return dlg.Open(DLG_TYPE_ASYNC,ID_LISTVIEWTEST,-1,-1);
}

Bool ListViewTest::RestoreLayout(void *secret)
{
	return dlg.RestoreLayout(ID_LISTVIEWTEST,0,secret);
}

Bool RegisterListView(void)
{
	return RegisterCommandPlugin(ID_LISTVIEWTEST,GeLoadString(IDS_LISTVIEW),0,NULL,String(),gNew ListViewTest);
}
