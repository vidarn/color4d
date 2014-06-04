/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code for a sub dialog

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_SUBDIALOGTEST 1000454

#include "c4d.h"
#include "c4d_symbols.h"


class MySubDialog1 : public SubDialog
{
	virtual Bool CreateLayout(void)
	{
		GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,2,0,"",BFV_GRIDGROUP_ALLOW_WEIGHTS);
			AddButton(1000,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog1");
			AddButton(1001,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog1");
			AddButton(1002,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog1");
			AddButton(1003,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog1");
			AddEditNumberArrows(1006,BFH_SCALEFIT);
			AddEditShortcut(1004,BFH_SCALEFIT|BFV_CENTER);

			BaseContainer fontchooserdata;
			fontchooserdata.SetBool( FONTCHOOSER_ENABLE_SIZE, TRUE );
			AddCustomGui( 1010, CUSTOMGUI_FONTCHOOSER, String(), BFH_SCALEFIT|BFV_SCALEFIT,0,0,fontchooserdata);

		GroupEnd();
		return TRUE;
	}
	virtual Bool InitValues(void)
	{
		SetLong(1006,LONG(0),0,10);

		BaseContainer shortcut;
		shortcut.SetLong(0,0); // qual
		shortcut.SetLong(1,KEY_F1); // key

		BaseContainer m(BFM_VALUECHNG);
		m.SetContainer(BFM_ACTION_VALUE,shortcut);

		SendMessage(1004,m);
		
		return TRUE;
	}
	virtual Bool Command(LONG id,const BaseContainer &msg)
	{
		switch (id)
		{
			case 1004:
				{
					const GeData &d = msg.GetData(BFM_ACTION_VALUE);
					if (d.GetType()==DA_CONTAINER)
					{
						const BaseContainer *bc = d.GetContainer();
						String shortcut;
						if (bc)
						{
							shortcut = Shortcut2String(*bc);
							GePrint("Shortcut received: "+shortcut);
						}
					}					
				}
				break;
		}
		return TRUE;
	}
};

class MySubDialog2 : public SubDialog
{
	BaseContainer weights;
	Bool weights_saved;

public:

	MySubDialog2()
	{
		weights_saved = FALSE;
	}

	virtual Bool CreateLayout(void)
	{
		GroupBegin(999,BFH_SCALEFIT|BFV_SCALEFIT,3,0,"",BFV_GRIDGROUP_ALLOW_WEIGHTS);

			AddStaticText(1000,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog2",BORDER_THIN_IN);
			AddStaticText(1001,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog2",BORDER_THIN_IN);
			AddStaticText(1001,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog2",BORDER_THIN_IN);

			AddStaticText(1002,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog2",BORDER_THIN_IN);
			AddStaticText(1003,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog2",BORDER_THIN_IN);
			AddStaticText(1003,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog2",BORDER_THIN_IN);

			AddStaticText(1004,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog2",BORDER_THIN_IN);
			AddStaticText(1005,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog2",BORDER_THIN_IN);
			AddStaticText(1005,BFH_SCALEFIT|BFV_SCALEFIT,0,0,"SubDialog2",BORDER_THIN_IN);

			AddEditNumberArrows(1006,BFH_SCALEFIT);

			if (!weights_saved)
			{
				// set the columns
				weights.SetLong(GROUPWEIGHTS_PERCENT_W_CNT,3); // number of rows - has to be equal to the given layout
				weights.SetReal(GROUPWEIGHTS_PERCENT_W_VAL+0,1); // weight for col 1
				weights.SetReal(GROUPWEIGHTS_PERCENT_W_VAL+1,-250); // FIXED weight for col 2
				weights.SetReal(GROUPWEIGHTS_PERCENT_W_VAL+2,1); // weight for col 1

				// set the rows
				weights.SetLong(GROUPWEIGHTS_PERCENT_H_CNT,4); // number of rows - has to be equal to the given layout
				weights.SetReal(GROUPWEIGHTS_PERCENT_H_VAL+0,-1.0); // weight for row 1
				weights.SetReal(GROUPWEIGHTS_PERCENT_H_VAL+1,-150.0); // FIXED weight for row 2
				weights.SetReal(GROUPWEIGHTS_PERCENT_H_VAL+2,60.0); // weight for row 3
				weights.SetReal(GROUPWEIGHTS_PERCENT_H_VAL+3,0.0); // weight for row 4
				weights_saved = TRUE;
			}

			GroupWeightsLoad(999,weights);

		GroupEnd();
		return TRUE;
	}
	virtual LONG Message(const BaseContainer &msg,BaseContainer &result)
	{
		switch (msg.GetId())
		{
			case BFM_WEIGHTS_CHANGED:
				// if the weights change because of user interaction you will get notified
				if (msg.GetLong(BFM_WEIGHTS_CHANGED)==999)
					GroupWeightsSave(999,weights);
				break;
		}
		return SubDialog::Message(msg,result);
	}
	virtual Bool InitValues(void)
	{
		SetLong(1006,LONG(0),0,10);
		return TRUE;
	}
	virtual Bool Command(LONG id,const BaseContainer &msg)
	{
		return TRUE;
	}
};


class MainDialog : public GeDialog
{
	private:
		MySubDialog1 subdialog1;
		MySubDialog2 subdialog2;
		SubDialog    *lastdlg;

	public:
		MainDialog(void);
		virtual ~MainDialog(void);

		virtual Bool CreateLayout(void);
		virtual Bool InitValues(void);
		virtual Bool Command(LONG id,const BaseContainer &msg);
		virtual LONG Message(const BaseContainer &msg,BaseContainer &result);
};

MainDialog::MainDialog(void)
{
	lastdlg = NULL;
}

MainDialog::~MainDialog(void)
{
}

Bool MainDialog::CreateLayout(void)
{
	// first call the parent instance
	Bool res = GeDialog::CreateLayout();

	res = LoadDialogResource(DLG_SUBDIALOG,NULL,0);

	if (res)
	{
		AttachSubDialog(&subdialog1,GADGET_SUBDIALOG);
		lastdlg = &subdialog1;
	}

	return res;
}

Bool MainDialog::InitValues(void)
{
	// first call the parent instance
	if (!GeDialog::InitValues()) return FALSE;

	return TRUE;
}

Bool MainDialog::Command(LONG id,const BaseContainer &msg)
{
	switch (id)
	{
		case GADGET_SUB1:
			if (!lastdlg || !lastdlg->CheckClose())
			{
				AttachSubDialog(&subdialog1,GADGET_SUBDIALOG);
				LayoutChanged(GADGET_SUBDIALOG);
				lastdlg = &subdialog1;
			}
			break;

		case GADGET_SUB2:
			if (!lastdlg || !lastdlg->CheckClose())
			{
				AttachSubDialog(&subdialog2,GADGET_SUBDIALOG);
				LayoutChanged(GADGET_SUBDIALOG);
				lastdlg = &subdialog2;
			}
			break;
	}
	return TRUE;
}


LONG MainDialog::Message(const BaseContainer &msg,BaseContainer &result)
{
//	switch (msg.GetId())
//	{
//	}
	return GeDialog::Message(msg,result);
}

class SubDialogTest : public CommandData
{
	private:
		MainDialog dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual LONG GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);
};

LONG SubDialogTest::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}

Bool SubDialogTest::Execute(BaseDocument *doc)
{
	return dlg.Open(DLG_TYPE_ASYNC,ID_SUBDIALOGTEST,-1,-1);
}

Bool SubDialogTest::RestoreLayout(void *secret)
{
	return dlg.RestoreLayout(ID_SUBDIALOGTEST,0,secret);
}

Bool RegisterSubDialog(void)
{
	return RegisterCommandPlugin(ID_SUBDIALOGTEST,GeLoadString(IDS_SUBDIALOG),0,NULL,String(),gNew SubDialogTest);
}
