#include "PaletteSubDialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "c4d_file.h"
#include "ase_loader.h"
#include <iconv.h>
#include <string>

PaletteSubDialog::PaletteSubDialog():
m_spotColors(NULL), m_selectCallback(NULL),m_dragable(TRUE),m_showLabel(FALSE)
{
}


Bool PaletteSubDialog::CreateLayout(void)
{
    if (!GeDialog::CreateLayout()) return FALSE;

    GroupBegin(0,BFH_SCALEFIT|BFV_SCALEFIT,0,1,String(),0);
		GroupBegin(1,BFV_SCALEFIT,0,1,String(),0);
            m_actionPopup = AddPopupButton(2,BFH_LEFT);
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
    if(m_showLabel){
        bc.SetString(ACTION_LABEL, "Show labels&c&");
    }else{
        bc.SetString(ACTION_LABEL, "Show labels");
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

Bool PaletteSubDialog::Command(Int32 id,const BaseContainer &msg)
{
	GeDynamicArray<Palette> pals;
    Palette pal;
    ASE_FILE aseFile;
    Filename fn;
    switch (id)
    {
		case 2:
            GePrint("Tjohoo!");
            switch(msg.GetInt32(BFM_ACTION_VALUE)){
                case ACTION_NEW:
                    pal.SetColor(0, Color(0.f, 0.f, 0.f).SetSource(COLOR_SOURCE_DISPLAY));
                    id = Palette::AddPalette(pal);
                    LoadPalette(id);
                    break;
                case ACTION_LOAD:
                    if(fn.FileSelect(FILESELECTTYPE_ANYTHING, FILESELECT_LOAD, "Load")){
                        String s = fn.GetString();
                        Int32 fnLength =  s.GetCStringLen();
                        char *str = NewMem(char,fnLength+1);
                        s.GetCString(str, fnLength+1);
                        ASE_ERRORTYPE error = ase_openAndReadAseFile(&aseFile, str);
                        DeleteMem(str);
                        if(!error){
                            for(Int32 i=0;i<aseFile.numGroups;i++){
                                ASE_GROUP *group = aseFile.groups + i;
#pragma message("TODO: This isn't nice at all!")
                                
                                size_t len = wcslen(group->name);
                                char *buffer = NewMem(char, len);
                                for(Int32 ii=0;ii<len;ii++){
                                    buffer[ii] = ((char *) group->name)[ii*4];
                                }
                                
                                GePrint(String("Name: ") + String(buffer,STRINGENCODING_UTF8));
                                pal.m_name = String(buffer,STRINGENCODING_UTF8);
                                DeleteMem(buffer);
                                
                                for(Int32 ii=0;ii<group->numColors;ii++){
                                    ASE_COLOR *color = group->colors + ii;
                                    Int32 a = 0;
                                    Vector col;
                                    while(color->col[a] != -1.0f && a < 3){
                                        printf("a: %d\n",a);
                                        GePrint(String::FloatToString(color->col[a]));
                                        col[a] = color->col[a];
                                        a++;
                                    }
                                    
                                    len = wcslen(color->name);
                                    buffer = NewMem(char, len);
                                    for(Int32 ii=0;ii<len;ii++){
                                        buffer[ii] = ((char *) color->name)[ii*4];
                                    }
                                    pal.SetColor(ii, NamedColor(Color(col).SetSource(COLOR_SOURCE_DISPLAY),String(buffer,STRINGENCODING_UTF8)));
                                    DeleteMem(buffer);
                                    GePrint(" ");
                                }
                                id = Palette::AddPalette(pal);
                                LoadPalette(id);
                                
                            }
                        } else {
                            GePrint("Could not load file " + s);
                        }
                    }
                    break;
                case ACTION_LABEL:
                    m_showLabel = !m_showLabel;
                    LoadPalette(m_paletteID);
                    break;
                    
            }
            if(msg.GetInt32(BFM_ACTION_VALUE) >= ACTION_COUNT){
                LoadPalette(msg.GetInt32(BFM_ACTION_VALUE)-ACTION_COUNT);
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
      case  PALETTE_ID:                                      // internal message
			Int64 color =  (Int64) msg.GetVoid( BFM_CORE_PAR1 );
			Int64 palette = (Int64) msg.GetVoid( BFM_CORE_PAR2 );
			if(palette == m_paletteID && color == -1){
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
    UpdatePopup(id);
	GeDynamicArray<Palette> pals;
	Palette::GetPalettes(pals);
	m_palette = pals[id];
	m_paletteID = id;
	PaletteLayout();
}

void PaletteSubDialog::PaletteLayout()
{
	LayoutFlushGroup(6);
	if(m_spotColors != NULL){
		delete[] m_spotColors;
	}
	m_spotColors = new PaletteColor[m_palette.GetCount()];
	GePrint("PaletteLayout: " + String::IntToString(m_palette.GetCount()));
	for(int i=0;i<m_palette.GetCount();i++){
		m_spotColors[i].SetColor(m_palette[i]);
		m_spotColors[i].SetColorID(i);
		m_spotColors[i].SetSelectCallback(m_selectCallback,m_selectCallbackData);
		m_spotColors[i].SetDragable(m_dragable);
        m_spotColors[i].SetPaletteID(m_paletteID);
        
        if(m_showLabel){
            GroupBegin(40 + i*3,BFV_FIT,1,0,"",FALSE);
            AddStaticText(40 + i*3+1,BFV_FIT,SizePix(40),0,m_palette[i].m_name,BORDER_NONE);
        }
        C4DGadget *area = AddUserArea(40 + i*3+2,BFV_SCALEFIT);
        AttachUserArea(m_spotColors[i],area);
        if(m_showLabel){
            GroupEnd();
        }

	}
	LayoutChanged(6);
}