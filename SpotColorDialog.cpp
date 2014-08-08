#include "spotcolordialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"

// TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO
//
// Make sure icc profiles and transforms are correctly destroyed!
//
// TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO-TODO


Bool SpotColorDialog::CreateLayout(void)
{
	m_displayProfile = Color::getDisplayProfile();

	GePrint("Create Layout!");
    BaseContainer *wprefs=GetWorldContainerInstance();

    m_Settings=wprefs->GetContainer(COLORPICKER_ID);

    if (!GeDialog::CreateLayout()) return FALSE;

    SetTitle(GeLoadString(IDS_COLORPICKER));

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,1,0,String(),0);
		GroupBegin(1,BFH_SCALEFIT,0,1,String(),0);
			AddStaticText(IDC_ICCLABEL,BFH_SCALEFIT,0,0,String("ICC Profile"),BORDER_NONE);
			iccSpotCombo = AddComboBox(IDC_SPOTICC,BFH_SCALEFIT,10,10);
			m_labelCheckbox = AddCheckbox(IDC_LABELCHECKBOX,BFH_SCALEFIT,0,0,String("Show names"));
		GroupEnd();
		m_searchText = AddEditText(IDC_SEARCHTEXT,BFH_SCALEFIT);
		ScrollGroupBegin(5,BFH_SCALEFIT|BFV_SCALEFIT,SCROLLGROUP_VERT);
			GroupBorderNoTitle(BORDER_THIN_IN);
			GroupBegin(6,BFH_SCALEFIT|BFV_SCALEFIT,7,0,String(),0);
			GroupEnd();
		GroupEnd();
    GroupEnd();

    return TRUE;
}

SpotColorDialog::~SpotColorDialog()
{
	if(m_iccSearchPaths!= NULL){
		delete m_iccSearchPaths;
	}
}

void SpotColorDialog::FindICCProfiles(){
	if(m_iccSearchPaths!= NULL){
		delete m_iccSearchPaths;
	}
	m_iccSearchPaths = new String[1];
	m_iccSearchPaths[0] = "C:\\Windows\\System32\\Spool\\Drivers\\Color\\";

	BaseContainer spotbc;

	cmsHPROFILE sRGBProfile;
	sRGBProfile = cmsCreate_sRGBProfile();

	BrowseFiles* bf = BrowseFiles::Alloc();

	Filename dir(m_iccSearchPaths[0]);
	bf->Init(dir,FALSE);
	int spotPos = m_spotProfiles.GetCount();

	if (bf)
	{
		while (bf->GetNext())
		{
			Filename fileName = bf->GetFilename();
			fileName.SetDirectory(dir);
			String str = fileName.GetString();
			Char *buffer = new Char[str.GetCStringLen()+1];
			str.GetCString(buffer,str.GetCStringLen()+1);
			cmsHPROFILE profile = cmsOpenProfileFromFile(buffer, "r");
			if(profile != NULL){
				cmsColorSpaceSignature sig = cmsGetColorSpace(profile);
				Int32 length = cmsGetProfileInfoASCII(profile,cmsInfoDescription,"en","US",NULL,0);
				Char *buffer2 = new Char[length];
				cmsGetProfileInfoASCII(profile,cmsInfoDescription,"en","US",buffer2,length);
				String info(buffer2);
				int pt = _cmsLCMScolorSpace(sig);
				cmsHTRANSFORM xform = cmsCreateTransform(profile,TYPE_NAMED_COLOR_INDEX,m_displayProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
				if(xform != NULL){
					cmsNAMEDCOLORLIST* colorList = cmsGetNamedColorList(xform);
					if(colorList != NULL){
						spotbc.SetString(spotPos,info);
						m_spotProfiles.Insert(profile,spotPos);
						spotPos++;
					}
				}
				delete buffer2;
			}
			delete buffer;
		}
		AddChildren(iccSpotCombo,spotbc);
		SetInt32(iccSpotCombo,0);
		Enable(iccSpotCombo,TRUE);
	}
	
	BrowseFiles::Free(bf);
}

void SpotColorDialog::LoadSpotColors(Int32 index)
{
	String filterString;
	GetString(m_searchText,filterString);

	Bool showLabels;
	GetBool(m_labelCheckbox,showLabels);
	LayoutFlushGroup(6);
	double RGB[3];
	Char name[256], prefix[33], suffix[33];
    if(m_spotProfiles.GetCount() > index){
        cmsHPROFILE profile = m_spotProfiles[index];
    #pragma message("TODO: CHECK THIS TRANSFORM")
        cmsHTRANSFORM xform = cmsCreateTransform(profile,TYPE_NAMED_COLOR_INDEX,m_displayProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
        if(xform != NULL){
            cmsNAMEDCOLORLIST* colorList = cmsGetNamedColorList(xform);
            if(colorList != NULL){
                Char name[256], prefix[33], suffix[33];
                Int32 numColors = cmsNamedColorCount(colorList);
                if(m_spotColors != NULL){
                    delete m_spotColors;
                }
                m_spotColors = new SpotColor[numColors];
                for(int i=0;i<numColors;i+=7){
    #pragma message("TODO: This looks too complicated... :/")
                    Int32 limit = i+7 < numColors ? i+7 : numColors;
                    for(int ii=i;ii<limit;ii++){
                        cmsNamedColorInfo(colorList,ii,name,prefix,suffix,NULL,NULL);
                        String fullName = String(name) + String(suffix);
                        Int32 pos;
                        if(!filterString.Content() || fullName.FindFirst(filterString,&pos)){
                            Color col;
                            cmsDoTransform(xform,&ii,RGB,1);
                            for(int a=0;a<3;a++){
                                col[a] = RGB[a];
                            }
                            col.SetSource(COLOR_SOURCE_DISPLAY);
                            m_spotColors[ii].SetParent(NULL);

                            if(showLabels){
                                GroupBegin(ii+IDC_LASTENTRY,BFH_SCALEFIT,1,0,fullName,FALSE);
                                //GroupBorder(BORDER_WITH_TITLE|BORDER_THIN_IN);
                                AddStaticText(ii*2+IDC_LASTENTRY+1,BFH_SCALEFIT,0,0,fullName,BORDER_NONE);
                            }
                            C4DGadget *area = AddUserArea(ii*2+IDC_LASTENTRY,BFH_SCALEFIT);
                            AttachUserArea(m_spotColors[ii],area);
                            m_spotColors[ii].UpdateColor(col);
                            if(showLabels){
                                GroupEnd();
                            }
                        }
                    }
                }
            }
        }
	}
	LayoutChanged(6);
}

Bool SpotColorDialog::InitValues(void)
{
	FindICCProfiles();
	LoadSpotColors(0);
    return TRUE;
}

Bool SpotColorDialog::Command(Int32 id,const BaseContainer &msg)
{
	Int32 val;
    switch (id){
		case IDC_SPOTICC:
		case IDC_LABELCHECKBOX:
		case IDC_SEARCHTEXT:
			GetInt32(iccSpotCombo,val);
			LoadSpotColors(val);
			break;
    }
    return GeDialog::Command(id,msg);
}

Bool SpotColorDialog::CoreMessage(Int32 id,const BaseContainer &msg)
{
    return GeDialog::CoreMessage(id,msg);
}

Int32 SpotColorDialog::Message(const BaseContainer& msg, BaseContainer& result)
{
    switch (msg.GetId())
    {
    case BFM_COLORCHOOSER_PARENTMESSAGE:
    {
        m_Settings.SetContainer(BFM_COLORCHOOSER_PARENTMESSAGE,msg);
        break;
    }
    }

    return GeDialog::Message(msg,result);
}


Int32 SpotColorCommand::GetState(BaseDocument *doc)
{
	return CMD_ENABLED;
}

Bool SpotColorCommand::Execute(BaseDocument *doc)
{
	return dlg.Open(DLG_TYPE_ASYNC,SPOTCOLOR_ID,-1,-1);
}

Bool SpotColorCommand::RestoreLayout(void *secret)
{
	return dlg.RestoreLayout(SPOTCOLOR_ID,0,secret);
}