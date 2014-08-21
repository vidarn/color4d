#include "PaletteSubDialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"
#include <iconv.h>
#include <string>

PaletteSubDialog::PaletteSubDialog(Int32 id):
m_spotColors(NULL), m_selectCallback(NULL),m_dragable(TRUE),m_showLabel(FALSE),m_id(id),m_showControls(FALSE), m_controlsShown(TRUE), m_rowArea(NULL), m_labelCheckArea(NULL), m_layoutArea(NULL),m_nameArea(NULL),m_rows(1),m_layout(0)
{
}


Bool PaletteSubDialog::CreateLayout(void)
{
    if (!GeDialog::CreateLayout()) return FALSE;

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
        GroupBegin(1, BFV_SCALEFIT, 0, 1, String(), 0);
            GroupBegin(3,BFV_SCALEFIT,1,0,String(),0);
            m_actionPopup = AddPopupButton(3,BFH_LEFT);
            m_trashArea = AddUserArea(4, BFH_CENTER);
            AttachUserArea(m_trash, m_trashArea);
            GroupEnd();
        GroupEnd();
		ScrollGroupBegin(2,BFH_SCALEFIT|BFV_SCALEFIT,SCROLLGROUP_HORIZ);
			GroupBegin(6,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
            m_spotColors = new PaletteColor[1];
            m_spotColors[0].SetColor(Color(0.f,0.f,0.f).SetSource(COLOR_SOURCE_DISPLAY));
            m_spotColors[0].SetColorID(0);
            m_spotColors[0].SetSelectCallback(m_selectCallback,m_selectCallbackData);
            m_spotColors[0].SetDragable(m_dragable);
            C4DGadget *area = AddUserArea(12,BFV_SCALEFIT);
            AttachUserArea(m_spotColors[0],area);
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
    LoadPalette(0);
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
                    Palette::UpdateAll();
                    return TRUE;
                case ACTION_LOAD:
                    if(fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_LOAD, "Load")){
                        String s = fn.GetString();
                        if(Palette::LoadASEFile(s, pal)){
                            id = Palette::AddPalette(pal);
                            m_controlsShown = FALSE;
                            LoadPalette(id);
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
            }
            return TRUE;
        case IDC_LAYOUT_DIRECTION:
            m_controlsShown = FALSE;
            PaletteLayout();
            break;
        case IDC_ROWS:
            PaletteLayout();
            break;
        case IDC_LABELCHECKBOX:
            GetBool(m_labelCheckArea, m_showLabel);
            LoadPalette(m_paletteID);
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
                m_paletteID = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_ID)->GetInt32(m_id);
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
    GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_ID)->SetInt32(m_id, m_paletteID);
	PaletteLayout();
    UpdatePopup(id);
}

void PaletteSubDialog::PaletteLayout()
{
    Int32 rows = 0;
    Int32 cols = 0;
    Int32 tmp = 0;
    
    if(m_rowArea != NULL){
        GetInt32(m_rowArea, m_rows);
    }
    if(m_layoutArea != NULL){
        GetInt32(m_layoutArea, m_layout);
    }
    if(m_labelCheckArea != NULL){
        GetBool(m_labelCheckArea,m_showLabel);
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
        
            m_controlsShown = TRUE;
            
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
        m_controlsShown = FALSE;
    }
    
    rows = m_rows;
    if(m_layout == 1){
        tmp = rows;
        rows = cols;
        cols = tmp;
    }
	LayoutFlushGroup(6);
	if(m_spotColors != NULL){
		delete[] m_spotColors;
	}
	m_spotColors = new PaletteColor[m_palette.GetCount()];
    GroupBegin(30, BFH_SCALEFIT|BFV_SCALEFIT, cols, rows, String(), 0);
	for(int i=0;i<m_palette.GetCount();i++){
		m_spotColors[i].SetColor(m_palette[i]);
		m_spotColors[i].SetColorID(i);
		m_spotColors[i].SetSelectCallback(m_selectCallback,m_selectCallbackData);
		m_spotColors[i].SetDragable(m_dragable);
        m_spotColors[i].SetPaletteID(m_paletteID);
        
        if(m_showLabel){
            GroupBegin(40 + i*3,BFV_FIT,1,0,"",FALSE);
            AddStaticText(40 + i*3+1,BFV_FIT,0,0,m_palette[i].m_name,BORDER_NONE);
        }
        C4DGadget *area = AddUserArea(40 + i*3+2,0);
        AttachUserArea(m_spotColors[i],area);
        if(m_showLabel){
            GroupEnd();
        }

	}
    GroupEnd();
	LayoutChanged(6);
}