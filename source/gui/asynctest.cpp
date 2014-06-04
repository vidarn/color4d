/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example code for a menu/manager plugin

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_ASYNCTEST 1000955

#include "c4d.h"
#include "gradientuserarea.h"
#include "c4d_symbols.h"
#include "lib_browser.h"

enum
{
	GADGET_ADDROW = 5000,
	GADGET_SUBROW,
	GADGET_R1,
	GADGET_R2,
	GROUP_DYNAMIC,
	GROUP_SCROLL,

	GADGET_DRAG = 6000,

	_dummy
};

class SDKGradientArea : public GeUserArea
{
	public:
		SDKGradientGadget ggtmp;
		SDKGradient	grad[MAXGRADIENT];
		LONG				count,interpolation,type;

		SDKGradientArea(void);

		virtual Bool Init(void);
		virtual Bool GetMinSize(LONG &w,LONG &h);
		virtual void Sized(LONG w,LONG h);
		virtual void DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg);
		virtual Bool InputEvent(const BaseContainer &msg);
};

SDKGradientArea::SDKGradientArea(void)
{
	LONG i;
	for (i=0; i<MAXGRADIENT; i++)
		grad[i].id=i;

	grad[0].col = Vector(1.0,1.0,0.0);
	grad[1].col = Vector(1.0,0.0,0.0);
	grad[0].pos = 0.0;
	grad[1].pos = 1.0;

	count=2;
	interpolation=4;
	type=0;
}

Bool SDKGradientArea::Init(void)
{
	ggtmp.Init(this,grad,&count,&interpolation,MAXGRADIENT);
	return TRUE;
}

Bool SDKGradientArea::GetMinSize(LONG &w,LONG &h)
{
	w = 100;
	h = 200;
	return TRUE;
}

void SDKGradientArea::Sized(LONG w,LONG h)
{
	ggtmp.InitDim(w,h);
}

void SDKGradientArea::DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg)
{
	// skip the redraw in case if focus change
	LONG reason = msg.GetLong(BFM_DRAW_REASON);
	if (reason==BFM_GOTFOCUS || reason==BFM_LOSTFOCUS) 
		return;

	if (!ggtmp.col) return;
	LONG w = ggtmp.col->GetBw();
	LONG h = ggtmp.col->GetBh();
	DrawBitmap(ggtmp.col,0,0,w,h,0,0,w,h,0);
}

Bool SDKGradientArea::InputEvent(const BaseContainer &msg)
{
	LONG dev = msg.GetLong(BFM_INPUT_DEVICE);
	LONG chn = msg.GetLong(BFM_INPUT_CHANNEL);
	if (dev==BFM_INPUT_MOUSE)
	{
		BaseContainer action(BFM_ACTION);
		action.SetLong(BFM_ACTION_ID,GetId());
		action.SetLong(BFM_ACTION_VALUE,0);

		if (chn==BFM_INPUT_MOUSELEFT)
		{
			LONG mxn,myn;
			LONG mx = msg.GetLong(BFM_INPUT_X);
			LONG my = msg.GetLong(BFM_INPUT_Y);
			Bool dc = msg.GetBool(BFM_INPUT_DOUBLECLICK);
			Global2Local(&mx,&my);

			if (ggtmp.MouseDown(mx,my,dc))
			{
				BaseContainer z;
				while (GetInputState(BFM_INPUT_MOUSE,BFM_INPUT_MOUSELEFT,z))
				{
					if (z.GetLong(BFM_INPUT_VALUE)==0) break;

					mxn = z.GetLong(BFM_INPUT_X);
					myn = z.GetLong(BFM_INPUT_Y);
					Global2Local(&mxn,&myn);

					mx=mxn; my=myn;
					ggtmp.MouseDrag(mx,my);
					Redraw();
					action.SetLong(BFM_ACTION_INDRAG,TRUE);
					SendParentMessage(action);
				}
			}
			Redraw();

			action.SetLong(BFM_ACTION_INDRAG,FALSE);
			SendParentMessage(action);
		}
		else if (chn==BFM_INPUT_MOUSEWHEEL)
		{
			Real per;
			if (ggtmp.GetPosition(&per))
			{
				per+=msg.GetLong(BFM_INPUT_VALUE)/120.0*0.01;
				per=FCut01(per);
				ggtmp.SetPosition(per);
				Redraw();
				SendParentMessage(action);
			}
		}
		return TRUE;
	}
	return FALSE;
}

class AsyncDialog : public GeDialog
{
	private:
		LONG rows;
		String array_drag[100];
		BaseContainer links;

		void DoEnable(void);
		Bool CheckDropArea(LONG id,const BaseContainer &msg);
		void CreateDynamicGroup(void);
		void ReLayout(void);
		String GetStaticText(LONG i);

		SDKGradientArea sg;
		C4DGadget				*gradientarea;

	public:
		AsyncDialog(void);
		virtual Bool CreateLayout(void);
		virtual Bool InitValues(void);
		virtual Bool Command(LONG id,const BaseContainer &msg);
		virtual LONG Message(const BaseContainer &msg,BaseContainer &result);
		virtual Bool CoreMessage  (LONG id,const BaseContainer &msg);
};

enum
{
	IDC_OFFSET		= 1001,
	IDC_ACCESS		= 1002,
	IDC_GRADTEST	= 1003,
	IDC_XPOSITION	= 1004,
	IDC_XINTERPOL	= 1005
};

AsyncDialog::AsyncDialog(void)
{
	gradientarea=NULL;
	rows = 1;
}

Bool AsyncDialog::CreateLayout(void)
{
	// first call the parent instance
	Bool res = GeDialog::CreateLayout();

	SetTitle("GuiDemo C++");

	GroupBegin(0,BFH_SCALEFIT,5,0,"",0);
	{
		GroupBorderSpace(4,4,4,4);
		AddButton(GADGET_ADDROW,BFH_FIT,0,0,"add row");
		AddButton(GADGET_SUBROW,BFH_FIT,0,0,"sub row");
	}
	GroupEnd();

	GroupBegin(0,BFH_SCALEFIT,2,0,"",0);
	{
		GroupBegin(0,BFV_SCALEFIT|BFH_SCALEFIT,0,1,"Drop objects, tags, materials here",0);
		{
			GroupBorder(BORDER_GROUP_IN|BORDER_WITH_TITLE);
			GroupBorderSpace(4,4,4,4);

			ScrollGroupBegin(GROUP_SCROLL,BFH_SCALEFIT|BFV_SCALEFIT,SCROLLGROUP_VERT);
			{
				GroupBegin(GROUP_DYNAMIC,BFV_TOP|BFH_SCALEFIT,3,0,"",0);
				{
					CreateDynamicGroup();
				}
				GroupEnd();
			}
			GroupEnd();
		}
		GroupEnd();

		GroupBegin(0,BFV_SCALEFIT|BFH_SCALEFIT,0,2,"",0);
		{
			gradientarea = AddUserArea(IDC_GRADTEST,BFH_SCALEFIT);	
			if (gradientarea) AttachUserArea(sg,gradientarea);

			GroupBegin(0,BFH_LEFT,2,0,"",0);
			{
				AddStaticText(0,BFH_LEFT,0,0,GeLoadString(IDS_INTERPOLATION),0);
				AddComboBox(IDC_XINTERPOL, BFH_SCALEFIT);
					IconData dat1,dat2,dat3,dat4;
					GetIcon(Ocube,&dat1);
					GetIcon(Osphere,&dat2);
					GetIcon(Ocylinder,&dat3);
					GetIcon(Ttexture,&dat4);
					AddChild(IDC_XINTERPOL, 0, GeLoadString(IDS_NONE)+"&"+PtrToString(&dat1)+"&");
					AddChild(IDC_XINTERPOL, 1, GeLoadString(IDS_LINEAR)+"&"+PtrToString(&dat2)+"&");
					AddChild(IDC_XINTERPOL, 2, GeLoadString(IDS_EXPUP)+"&"+PtrToString(&dat3)+"&");
					AddChild(IDC_XINTERPOL, 3, GeLoadString(IDS_EXPDOWN)+"&"+PtrToString(&dat4)+"&");
					AddChild(IDC_XINTERPOL, 4, GeLoadString(IDS_SMOOTH)+"&i"+LongToString(Tphong)+"&"); // use Icon ID

				AddStaticText(0,BFH_LEFT,0,0,GeLoadString(IDS_POSITION),0);
				AddEditNumberArrows(IDC_XPOSITION,BFH_LEFT);
			}
			GroupEnd();
		}
		GroupEnd();
	}
	GroupEnd();

	MenuFlushAll();	
		MenuSubBegin("Menu1");
			MenuAddString(GADGET_ADDROW,"add row");
			MenuAddString(GADGET_SUBROW,"sub row");
			MenuAddString(GADGET_R1,"test1&c&");
			MenuAddString(GADGET_R2,"test2&d&");
			MenuAddSeparator();
			MenuSubBegin("SubMenu1");
				MenuAddCommand(1001153); // atom object
				MenuAddCommand(1001157); // rounded tube object
				MenuAddCommand(1001158); // spherify object
				MenuSubBegin("SubMenu2");
					MenuAddCommand(1001154); // double circle object
					MenuAddCommand(1001159); // triangulate object
				MenuSubEnd();
			MenuSubEnd();
		MenuSubEnd();
	MenuFinished();

		GroupBeginInMenuLine();
		AddCheckbox(50000,0,0,0,String("Test"));
	GroupEnd();

	return res;
}

void AsyncDialog::DoEnable(void)
{
	Real pos=0.0;
	Bool ok=sg.ggtmp.GetPosition(&pos);
	Enable(IDC_XPOSITION,ok);
	Enable(IDC_XINTERPOL,ok);
}

void AsyncDialog::ReLayout(void)
{
	LayoutFlushGroup(GROUP_DYNAMIC);
	CreateDynamicGroup();
	LayoutChanged(GROUP_DYNAMIC);
}

Bool AsyncDialog::InitValues(void)
{
	// first call the parent instance
	if (!GeDialog::InitValues()) return FALSE;

	SetLong(IDC_OFFSET,100,0,100,1);
	SetBool(IDC_ACCESS,TRUE);

	Real pos=0.0;
	sg.ggtmp.GetPosition(&pos);

	SetPercent(IDC_XPOSITION,pos);
	SetLong(IDC_XINTERPOL,sg.interpolation);

	DoEnable();

	return TRUE;
}

Bool AsyncDialog::CheckDropArea(LONG id,const BaseContainer &msg)
{
	LONG x,y,w,h,dx,dy;
	GetDragPosition(msg,&dx,&dy);
	GetItemDim(id,&x,&y,&w,&h);
	return dy>y && dy<y+h;
}

void AsyncDialog::CreateDynamicGroup(void)
{
	LONG i;
	for (i=0;i<rows;i++)
	{
		AddCheckbox(0,BFH_LEFT,0,0,"Rows "+LongToString(i+1));
		AddStaticText(GADGET_DRAG+i,BFH_SCALEFIT,260,0,GetStaticText(i),BORDER_THIN_IN);

		AddEditSlider(0,BFH_SCALEFIT,0,0);
	}
}

Bool AsyncDialog::Command(LONG id,const BaseContainer &msg)
{
	switch (id)
	{
		case GADGET_ADDROW:
			if (rows<100) rows++;
			ReLayout();
			break;

		case GADGET_SUBROW:
			if (rows>1)
			{
				rows--;
				ReLayout();
			}
			break;

		case IDC_GRADTEST:
			InitValues();
			break;

		case IDC_XPOSITION:
			sg.ggtmp.SetPosition(msg.GetReal(BFM_ACTION_VALUE));
			sg.Redraw();
			break;

		case IDC_XINTERPOL:
			sg.interpolation = msg.GetLong(BFM_ACTION_VALUE);
			sg.ggtmp.CalcImage();
			sg.Redraw();
			break;
	}
	return TRUE;
}

static String GenText(C4DAtomGoal *bl)
{
	String str;
	if (bl->IsInstanceOf(Obase))
		str = "BaseObject";
	else if (bl->IsInstanceOf(Tbase))
		str = "BaseTag";
	else if (bl->IsInstanceOf(Mbase))
		str = "BaseMaterial";
	else if (bl->IsInstanceOf(CKbase))
		str = "CKey";
	else if (bl->IsInstanceOf(CTbase))
		str = "CTrack";
	else if (bl->IsInstanceOf(GVbase))
		str = "BaseNode";
	else
		return "Unknown object";

	if (bl->IsInstanceOf(Tbaselist2d))
		return str+" "+((BaseList2D*)bl)->GetName()+" ("+LongToString(bl->GetType())+")";

	return str+" ("+LongToString(bl->GetType())+")";
}

String AsyncDialog::GetStaticText(LONG i)
{
	C4DAtomGoal *bl = links.GetData(i).GetLinkAtom(GetActiveDocument());
	if (!bl) return String();
	return String("Dropped ")+GenText(bl);
}

Bool AsyncDialog::CoreMessage(LONG id,const BaseContainer &msg)
{
	switch (id)
	{
		case EVMSG_CHANGE:
			if (CheckCoreMessage(msg))
			{
				LONG i;
				for (i=0;i<rows;i++)
				{
					SetString(GADGET_DRAG+i,GetStaticText(i));
				}
			}
			break;
	}
	return GeDialog::CoreMessage(id,msg);
}

LONG AsyncDialog::Message(const BaseContainer &msg,BaseContainer &result)
{
	switch (msg.GetId())
	{
		case BFM_GETCURSORINFO:
			{
				// pluginprogrammers can return a help id in cursorinfo so that the onlinehelpsystem displays the help for a dialog
				HandleHelpString(msg,result,"PLUGIN_CMD_???");
			}
			break;

		case BFM_DRAGRECEIVE:
			{
				String prefix = "Dragging ";
				LONG i,id=-1;
				if (msg.GetLong(BFM_DRAG_FINISHED)) prefix = "Dropped ";

				if (CheckDropArea(GROUP_SCROLL,msg))
				{
					for (i=0;i<rows;i++)
					{
						if (CheckDropArea(GADGET_DRAG+i,msg)) { id = i; break; }
					}
				}
				if (id!=-1)
				{
					if (msg.GetLong(BFM_DRAG_LOST))
					{
						for (i=0;i<rows;i++)
						{
							SetString(GADGET_DRAG+i,GetStaticText(i));
						}
					}
					else
					{
						String string,str;
						LONG type = 0;
						void *object = NULL;
						C4DAtomGoal *bl = NULL;
						
						GetDragObject(msg,&type,&object);

						if (type==DRAGTYPE_ATOMARRAY && ((AtomArray*)object)->GetCount()==1 && ((AtomArray*)object)->GetIndex(0))
						{
							bl = (C4DAtomGoal*)((AtomArray*)object)->GetIndex(0);
							if (bl->IsInstanceOf(Obase))
							{
								str = "BaseObject";
							}
							else if (bl->IsInstanceOf(Tbase))
							{
								str = "BaseTag";
							}
							else if (bl->IsInstanceOf(Mbase))
							{
								str = "BaseMaterial";
							}
							else if (bl->IsInstanceOf(CKbase))
							{
								str = "CKey";
							}
							else if (bl->IsInstanceOf(CTbase))
							{
								str = "CTrack";
							}
	
							if (bl && bl->IsInstanceOf(Tbaselist2d))
								string = prefix+str+" "+((BaseList2D*)bl)->GetName()+" ("+LongToString(bl->GetType())+")";
							else
								string = prefix+str+" ("+LongToString(bl->GetType())+")";
						}
						else if ( type==DRAGTYPE_BROWSER )
						{
							SDKBrowserDragInfo	*bdi;
							
							bdi = (SDKBrowserDragInfo *) object;
							if ( bdi )
							{
								LONG	cnt;
								
								cnt = bdi->GetItemCount();
								if ( cnt > 1 )
									string = prefix + LongToString( cnt ) + " browser object(s)";
								else if ( cnt == 1 )
								{				
									SDKBrowserContentNodeRef	item;
									SDKBrowserPluginRef	plugin;
									
									item = bdi->GetItem( 0 );
									plugin = item->GetPlugin();
									string = prefix + "browser object " + item->GetName() + " " + plugin->GetTypeName( item, 0, SDKBrowserPluginInterface::SpecificItemType );
 								
 									if ( item->IsLink())											// is this a link to another node?
 									{
 										SDKBrowserContentNodeRef	link;
 										
 										link = SDKBrowser::FindNode( item->GetNodeURL( LinkThrough ));	// try to find the linked node
 										
 										if ( link )
 										{
 											item = link;
											plugin = item->GetPlugin();						// now get the real plugin (you could have used SDKBrowserContentNode::LinkThrough on the original as well);
 										}
 										else
 											string += " is a dead link";
 									}
 									
 									if ( item->GetTypeID() == SDKBrowserContentNode::TypePreset )	// is this some kind of preset?
 									{
 										string += " is a preset";
 										
 										if ( msg.GetLong(BFM_DRAG_FINISHED ))		// drag finished? Then get the preset data
 										{
	 										switch ( plugin->GetPluginID())
	 										{
												case	CBPluginTypeObjectPreset:
												{
													BaseDocument	*doc;
													
													doc = item->GetObjectPreset();
													if ( doc )
														BaseDocument::Free( doc );
													break;
												}
												case	CBPluginTypeMaterialPreset:
												{
													BaseMaterial	*mat;

													mat = item->GetMaterialPreset();
													if ( mat )
														BaseMaterial::Free( mat );
													break;
												}
												case	CBPluginTypeTagPreset:
												{
													BaseTag	*tag;

													tag = item->GetTagPreset();
													if ( tag )
														BaseTag::Free( tag );
													break;
												}
												case	CBPluginTypeRenderDataPreset:
												{
													RenderData	*rd;

													rd = item->GetRenderDataPreset();
													if ( rd )
														RenderData::Free( rd );
													break;
												}
												case	CBPluginTypeShaderPreset:
												{
													BaseShader	*ps;

													ps = item->GetShaderPreset();
													if ( ps )
														BaseShader::Free( ps );
													break;
												}
												case	CBPluginTypeVideoPostPreset:
												{
													BaseVideoPost	*vp;

													vp = item->GetVideoPostPreset();
													if ( vp )
														BaseVideoPost::Free( vp );
													break;
												}
	 										}
	 									}
									}
 								}
							}
						}
						else
							string = prefix+"unknown object";

						if (msg.GetLong(BFM_DRAG_FINISHED))
							links.SetLink(id,bl);

						for (i=0;i<rows;i++)
							array_drag[i] = GetStaticText(i);
						array_drag[id] = string;

						for (i=0;i<rows;i++)
							SetString(GADGET_DRAG+i,array_drag[i]);

						return SetDragDestination(MOUSE_POINT_HAND);
					}
				}
			}
			break;
	}
	return GeDialog::Message(msg,result);
}

class AsyncTest : public CommandData
{
	private:
		AsyncDialog dlg;
	public:
		virtual Bool Execute(BaseDocument *doc);
		virtual LONG GetState(BaseDocument *doc);
		virtual Bool RestoreLayout(void *secret);

//		virtual Bool ExecuteSubID(BaseDocument *doc, LONG subid);
//		virtual Bool GetSubContainer(BaseDocument *doc, BaseContainer &submenu);
};

LONG AsyncTest::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}

Bool AsyncTest::Execute(BaseDocument *doc)
{
	return dlg.Open(DLG_TYPE_ASYNC_FULLSCREEN_MONITOR,ID_ASYNCTEST,0,0);
}

Bool AsyncTest::RestoreLayout(void *secret)
{
	return dlg.RestoreLayout(ID_ASYNCTEST,0,secret);
}

//Bool AsyncTest::ExecuteSubID(BaseDocument *doc, LONG subid)
//{
//	switch (subid)
//	{
//		case 1000: GeOutString("Hello",GEMB_OK); break;
//		case 1001: GeOutString("World",GEMB_OK); break;
//	}
//	return TRUE;
//}

//Bool AsyncTest::GetSubContainer(BaseDocument *doc, BaseContainer &submenu)
//{
//	submenu.SetString(1000,String("Hello"));
//	submenu.SetString(1001,String("World"));
//	return TRUE;
//}

Bool RegisterAsyncTest(void)
{
	return RegisterCommandPlugin(ID_ASYNCTEST,GeLoadString(IDS_ASYNCTEST),0,NULL,String("C++ SDK Menu Test Plugin"),gNew AsyncTest);
}

