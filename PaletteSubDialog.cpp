#include "PaletteSubDialog.h"
#include "paletteshader.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"
#include "xpalette.h"
#include <iconv.h>
#include <string>

PaletteSubDialog::PaletteSubDialog(Int32 id):
m_spotColors(NULL), m_selectCallback(NULL),m_dragable(TRUE),m_showLabel(FALSE),m_id(id),m_showControls(FALSE), m_controlsShown(TRUE), m_rowArea(NULL), m_labelCheckArea(NULL), m_layoutArea(NULL),m_nameArea(NULL),m_searchText(NULL),m_rows(1),m_layout(0),m_searchString(""),m_paletteID(0)
{
}


Bool PaletteSubDialog::CreateLayout(void)
{
    if (!GeDialog::CreateLayout()) return FALSE;
    
    BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID)->GetContainerInstance(m_id);
    if(bc != nullptr){
        FromContainer(*bc);
    }else{
        m_paletteID = 0;
    }
    
    Int32 numRows = 1;
    if(m_layout == 0){
        numRows = m_rows;
    }

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
        GroupBegin(1, BFH_LEFT|BFV_TOP, 0, 1, String(), 0);
            GroupBegin(3,BFH_LEFT|BFV_TOP,1,0,String(),0);
            m_actionPopup = AddPopupButton(3,BFH_LEFT);
            m_trashArea = AddUserArea(4, BFH_CENTER);
            AttachUserArea(m_trash, m_trashArea);
            GroupEnd();
        GroupEnd();
        GroupBegin(8,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
            ScrollGroupBegin(2,BFH_SCALEFIT|BFV_SCALEFIT,SCROLLGROUP_VERT|SCROLLGROUP_HORIZ,SizePix(40*5),SizePix(40*numRows));
                GroupBegin(6,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
                m_spotColors = new PaletteColor[1];
                m_spotColors[0].SetColor(Color(0.f,0.f,0.f).SetSource(COLOR_SOURCE_DISPLAY));
                m_spotColors[0].SetColorID(0);
                m_spotColors[0].SetSelectCallback(m_selectCallback,m_selectCallbackData);
                m_spotColors[0].SetDragable(m_dragable);
                C4DGadget *area = AddUserArea(12,0);
                AttachUserArea(m_spotColors[0],area);
                GroupEnd();
            GroupEnd();
        GroupEnd();
 
    GroupEnd();
    return TRUE;
}

PaletteSubDialog::~PaletteSubDialog()
{
}

Bool PaletteSubDialog::InitValues(void)
{
    LoadPalette(m_paletteID);
    return TRUE;
}

void PaletteSubDialog::UpdatePopup(Int32 current)
{
    BaseContainer bc;
    bc.SetString(ACTION_LOAD, "Load palette");
    bc.SetString(ACTION_NEW,  "New palette");
    bc.SetString(ACTION_SAVE, "Save palette");
    bc.InsData(0, String(""));
    if(m_showControls){
        bc.SetString(ACTION_LABEL, "Show controls&c&");
    }else{
        bc.SetString(ACTION_LABEL, "Show controls");
    }
    bc.InsData(0, String(""));
    GeDynamicArray<Palette> pals;
    Palette::GetPalettes(pals);
    for(Int32 i=0;i<pals.GetCount();i++){
        String str;
        if(i==current){
            str = "&c&";
        }
        bc.SetString(ACTION_COUNT+i,pals[i].m_name+str);
    }
    SetPopup(m_actionPopup,bc);

}

void PaletteSubDialog::ShowControls(Bool show)
{
    m_showControls = show;
    LoadPalette(m_paletteID);
}

void PaletteSubDialog::ToContainer(BaseContainer &bc)
{
    Int32 i=0;
    bc.SetBool(++i, m_showLabel);
    bc.SetInt32(++i, m_rows);
    bc.SetInt32(++i, m_layout);
    bc.SetString(++i, m_searchString);
    bc.SetInt32(++i, m_paletteID);
    GePrint("Saving " + String::IntToString(m_paletteID));
}

void PaletteSubDialog::FromContainer(const BaseContainer &bc)
{
    Int32 i=0;
    m_showLabel    = bc.GetBool(++i);
    m_rows         = bc.GetInt32(++i);
    m_layout       = bc.GetInt32(++i);
    m_searchString = bc.GetString(++i);
    m_paletteID    = bc.GetInt32(++i);
    
    GePrint("Loading " + String::IntToString(m_paletteID));
    
    if(m_rowArea != NULL){
        SetInt32(m_rowArea, m_rows);
    }
    if(m_layoutArea != NULL){
        SetInt32(m_layoutArea, m_layout);
    }
    if(m_labelCheckArea != NULL){
        SetBool(m_labelCheckArea,m_showLabel);
    }
    if(m_searchText != NULL){
        SetString(m_searchText, m_searchString);
    }
    
    m_controlsShown = !m_showControls;
}

void PaletteSubDialog::SaveSettings()
{
    BaseContainer bc;
    ToContainer(bc);
    GePrint("Set container!");
    GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID)->SetContainer(m_id, bc);
}

Bool PaletteSubDialog::Command(Int32 id,const BaseContainer &msg)
{
	GeDynamicArray<Palette> pals;
    Palette pal;
    Filename fn;
    switch (id)
    {
		case 3:
            switch(msg.GetInt32(BFM_ACTION_VALUE)){
                case ACTION_NEW:
                    pal.SetColor(0, Color(0.f, 0.f, 0.f).SetSource(COLOR_SOURCE_DISPLAY));
                    id = Palette::AddPalette(pal);
                    m_controlsShown = FALSE;
                    LoadPalette(id);
                    SaveSettings();
                    Palette::UpdateAll();
                    return TRUE;
                case ACTION_LOAD:
                    if(fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_LOAD, "Load")){
                        String s = fn.GetString();
                        if(Palette::LoadASEFile(s, pal)){
                            id = Palette::AddPalette(pal);
                            m_controlsShown = FALSE;
                            LoadPalette(id);
                            SaveSettings();
                            Palette::UpdateAll();
                        }
                    }
                    return TRUE;
                case ACTION_SAVE:
                    if(fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_SAVE, "Save", "ase")){
                        String s = fn.GetString();
                        Palette::SaveASEFile(s, m_palette);
                    }
                    return TRUE;
                case ACTION_LABEL:
                    ShowControls(!m_showControls);
                    return TRUE;
                    
            }
            if(msg.GetInt32(BFM_ACTION_VALUE) >= ACTION_COUNT){
                m_controlsShown = FALSE;
                LoadPalette(msg.GetInt32(BFM_ACTION_VALUE)-ACTION_COUNT);
                SaveSettings();
            }
            return TRUE;
        case IDC_LAYOUT_DIRECTION:
            m_controlsShown = FALSE;
            LoadPalette(m_paletteID);
            SaveSettings();
            break;
        case IDC_ROWS:
            PaletteLayout();
            SaveSettings();
            break;
        case IDC_LABELCHECKBOX:
            GetBool(m_labelCheckArea, m_showLabel);
            LoadPalette(m_paletteID);
            SaveSettings();
            break;
        case IDC_NAME:
            if(m_nameArea != NULL){
                GetString(m_nameArea, m_palette.m_name);
                Palette::SetPaletteName(m_paletteID, m_palette.m_name);
                LoadPalette(m_paletteID);
            }
            break;
        case IDC_HIDE:
            ShowControls(FALSE);
            break;
        case IDC_SEARCHTEXT:
            PaletteLayout();
            SaveSettings();
            break;
        case IDC_CREATEMATERIAL:
            {
                GePrint("Palette ID: " + String::IntToString(m_paletteID));
                for(Int32 i=m_palette.m_colors.GetCount()-1;i>=0;--i){
                    BaseMaterial *mat = BaseMaterial::Alloc(Mmaterial);
                    String name = "PaletteMaterial";
                    if(m_palette[i].m_name != ""){
                        name = m_palette[i].m_name;
                    }
                    mat->SetName(name);
                    if(mat != nullptr){
                        BaseChannel *chan = mat->GetChannel(CHANNEL_COLOR);
                        if(chan != nullptr){
                            BaseContainer bc;
                            bc.SetInt32(BASECHANNEL_SHADERID, PALETTE_SHADER_ID);
                            chan->SetData(bc);
                            BaseShader *bs = chan->GetShader();
                            BaseContainer* data = bs->GetDataInstance();
                            data->SetInt32(PALETTESHADER_PALETTE_ID, 1+m_paletteID);
                            data->SetInt32(PALETTESHADER_COLOR_ID, 1+i);
                            GetActiveDocument()->InsertMaterial(mat);
                            EventAdd();
                        }
                    }
                }
            }
            break;
		default:
			break;
    }
    return GeDialog::Command(id,msg);
}

Bool PaletteSubDialog::CoreMessage(Int32 id, const BaseContainer& msg)
{
    switch ( id )
    {
      case  PALETTE_ID:
			Int64 color =  (Int64) msg.GetVoid( BFM_CORE_PAR1 );
			Int64 palette = (Int64) msg.GetVoid( BFM_CORE_PAR2 );
            if( palette == -1){
                BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID)->GetContainerInstance(m_id);
                if(bc != nullptr){
                    FromContainer(*bc);
                } else {
                    BaseContainer tmp;
                    m_paletteID = 0;
                    m_rows = 1;
                    m_layout = 0;
                    m_searchString = "";
                    m_showLabel = FALSE;
                    ToContainer(tmp);
                    GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID)->SetContainer(m_id,tmp);
                    FromContainer(tmp);
                }
            }
			if((palette == m_paletteID || palette == -1) && color == -1){
				LoadPalette(m_paletteID);
			}
        break;
    }
    return GeDialog::CoreMessage( id, msg );
}

void PaletteSubDialog::SetSelectCallback(void (*selectCallback)(Color,void *), void *data)
{
	m_selectCallback = selectCallback;
	m_selectCallbackData = data;
	for(int i=0;i<m_palette.GetCount();i++){
		m_spotColors[i].SetSelectCallback(selectCallback,data);
	}
}

void PaletteSubDialog::SetDragable(Bool state)
{
	m_dragable = state;
	for(int i=0;i<m_palette.GetCount();i++){
		m_spotColors[i].SetDragable(state);
	}
}

void PaletteSubDialog::LoadPalette(Int32 id)
{
	GeDynamicArray<Palette> pals;
	Palette::GetPalettes(pals);
	m_palette = pals[id];
    m_palette.m_name = pals[id].m_name;
	m_paletteID = id;
	PaletteLayout();
    UpdatePopup(id);
}

void PaletteSubDialog::LayoutPalette()
{
    Int32 rows = 0;
    Int32 cols = 0;
    Int32 tmp = 0;
    rows = m_rows;
    if(m_layout == 1){
        tmp = rows;
        rows = cols;
        cols = tmp;
    }
    if(m_spotColors != NULL){
        delete[] m_spotColors;
    }
    m_spotColors = new PaletteColor[m_palette.GetCount()];
    GroupBegin(30, BFH_SCALEFIT|BFV_SCALEFIT, cols, rows, String(), 0);
    GroupSpace(0,0);
    for(int i=0;i<m_palette.GetCount();i++){
        Int32 pos;
        if(m_searchString == "" || m_palette[i].m_name.ToLower().FindFirst(m_searchString.ToLower(), &pos)){
            m_spotColors[i].SetColor(m_palette[i]);
            m_spotColors[i].SetColorID(i);
            m_spotColors[i].SetSelectCallback(m_selectCallback,m_selectCallbackData);
            m_spotColors[i].SetDragable(m_dragable);
            m_spotColors[i].SetPaletteID(m_paletteID);
            
            if(m_showLabel){
                GroupBegin(400 + i*3,0,1,0,"",FALSE);
                AddStaticText(400 + i*3+1,0,0,0,m_palette[i].m_name,BORDER_NONE);
            }
            C4DGadget *area = AddUserArea(400 + i*3+2,0);
            AttachUserArea(m_spotColors[i],area);
            if(m_showLabel){
                GroupEnd();
            }
        }
    }
    GroupEnd();
}

void PaletteSubDialog::PaletteLayout()
{
    if(m_rowArea != NULL){
        GetInt32(m_rowArea, m_rows);
    }
    if(m_layoutArea != NULL){
        GetInt32(m_layoutArea, m_layout);
    }
    if(m_labelCheckArea != NULL){
        GetBool(m_labelCheckArea,m_showLabel);
    }
    if(m_searchText != NULL){
        GetString(m_searchText, m_searchString);
    }
    if(m_showControls && !m_controlsShown){
        LayoutFlushGroup(1);
        GroupBegin(51, BFV_SCALEFIT, 1, 0, String(), 0);
            m_nameArea = AddEditText(IDC_NAME, BFH_SCALEFIT);
            SetString(IDC_NAME, m_palette.m_name);
            
            GroupBegin(123,BFH_SCALEFIT,0,1,String(),0);
            String rowText("Rows");
            if(m_layout == 1){
                GePrint("Columns");
                rowText = String("Columns");
            }
            GePrint(rowText);
            AddStaticText(9, BFH_LEFT, 0, 0, rowText, 0);
            
            m_rowArea = AddEditNumberArrows(IDC_ROWS, BFH_RIGHT);
            GroupEnd();
            SetInt32(m_rowArea, m_rows,1,99);
            
            m_layoutArea = AddComboBox(IDC_LAYOUT_DIRECTION,BFH_LEFT);
            AddChild(m_layoutArea, 0, String("Horizontal"));
            AddChild(m_layoutArea, 1, String("Vertical"));
            SetInt32(m_layoutArea, m_layout);
            
            m_labelCheckArea = AddCheckbox(IDC_LABELCHECKBOX,BFH_LEFT,0,0,String("Show Labels"));
            SetBool(m_labelCheckArea, m_showLabel);
        
            AddStaticText(IDC_FILTERLABEL, BFH_CENTER, 0, 0, String("Filter:"), 0);
            m_searchText = AddEditText(IDC_SEARCHTEXT, BFH_SCALEFIT);
            SetString(m_searchText, m_searchString);
        
            m_controlsShown = TRUE;
        
            AddButton(IDC_CREATEMATERIAL, BFH_CENTER, 0, 0, String("Create Materials"));
        
            AddButton(IDC_HIDE, BFH_CENTER, 0, 0, String("Hide Controls"));
        GroupEnd();
        
        GroupBegin(3,BFV_SCALEFIT,1,0,String(),0);
            m_actionPopup = AddPopupButton(3,BFH_LEFT);
            m_trashArea = AddUserArea(4, BFH_CENTER);
            AttachUserArea(m_trash, m_trashArea);
        GroupEnd();
        LayoutChanged(1);
    }
    if(!m_showControls && m_controlsShown){
        LayoutFlushGroup(1);
            GroupBegin(3,BFV_SCALEFIT,1,0,String(),0);
                m_actionPopup = AddPopupButton(3,BFH_LEFT);
                m_trashArea = AddUserArea(4, BFH_CENTER);
                AttachUserArea(m_trash, m_trashArea);
            GroupEnd();
        LayoutChanged(1);
        m_rowArea = NULL;
        m_layoutArea = NULL;
        m_labelCheckArea = NULL;
        m_searchText = NULL;
        m_controlsShown = FALSE;
    }
    
	LayoutFlushGroup(6);
    LayoutPalette();
	LayoutChanged(6);
}