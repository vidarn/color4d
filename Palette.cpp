#include "palette.h"
#define ASE_NO_UTF8
#include "ase_loader.h"
#include "ase_writer.h"
#include "utils.h"

Palette::Palette():
	m_name("Unnamed"),m_inScene(true)
{
}

Palette::Palette(String name, Int32 numColors):
	m_name(name), m_colors(numColors),m_inScene(true)
{
}

Palette::Palette(const Palette& pal):
	m_name(pal.m_name), m_inScene(pal.m_inScene)
{
	Int32 count = pal.m_colors.GetCount();
	m_colors.SetCount(0);
	for(int i=0;i<count;i++){
		m_colors.Push(pal.m_colors[i]);
	}
}

void Palette::ToContainer(BaseContainer &bc) const
{
	Int32 count = m_colors.GetCount();
	bc.SetInt32(NUM_COLORS,m_colors.GetCount());
	for(int i=0;i<count;i++){
		bc.SetVector(FIRST_COLOR+i*2,m_colors[i].Convert(COLOR_SOURCE_DISPLAY).AsVector());
        bc.SetString(FIRST_COLOR+i*2+1,m_colors[i].m_name);
	}
	bc.SetString(PALETTE_NAME,m_name);
}

void Palette::FromContainer(const BaseContainer &bc)
{
	Int32 count = bc.GetInt32(NUM_COLORS);
	m_colors.SetCount(0);
	for(int i=0;i<count;i++){
		m_colors.Push(NamedColor(Color(bc.GetVector(FIRST_COLOR+i*2)).SetSource(COLOR_SOURCE_DISPLAY),bc.GetString(FIRST_COLOR+i*2+1)));
	}
	m_name = bc.GetString(PALETTE_NAME,"Unnamed");
}

void Palette::SetColor(Int32 index, const Vector &color, COLOR_SOURCE source)
{
	SetColor(index,Color(color).SetSource(source));
}

void Palette::SetColor(Int32 index, const Color &color)
{
    String name = "";
    if(index < m_colors.GetCount()){
        name = m_colors[index].m_name;
    }
	SetColor(index, NamedColor(color,name));
}

void Palette::SetColor(Int32 index, const NamedColor &color)
{
	while(index >= m_colors.GetCount()){
		m_colors.Push(NamedColor());
	}
	m_colors[index] = color;
    m_colors[index].m_name = color.m_name;
}

void Palette::InsertColor(Int32 index, const Color &color)
{
	if(index >= m_colors.GetCount()){
		while(index >= m_colors.GetCount()){
			m_colors.Push(NamedColor());
		}
		m_colors[index] = NamedColor(color,"");
	}
	else{
		m_colors.Insert(NamedColor(color,""),index);
	}
}

void Palette::RemoveColor(Int32 index)
{
    if(m_colors.GetCount() == 1){
        m_colors[0] = NamedColor();
    }else{
        if(index < m_colors.GetCount()){
            m_colors.Remove(index);
        }
    }
}

const Palette &Palette::operator=(const Palette &pal)
{
	m_colors.SetCount(0);
	Int32 count = pal.m_colors.GetCount();
	for(int i=0;i<count;i++){
		m_colors.Push(pal.m_colors[i]);
	}
	return *this;
}

//--------------------------

void Palette::InitPalettes()
{
    BaseDocument *doc = GetActiveDocument();
    if(doc->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID) == nullptr){
        BaseContainer bc;
        bc.SetInt32(NUM_PALETTES,1);
        BaseContainer palC;
        Palette stdPal;
        Filename fName = GeGetPluginPath() + Filename("default.ase");
        if(!Palette::LoadASEFile(fName.GetString(), stdPal)){
            stdPal = Palette(String("Default"),1);
            stdPal.SetColor(0,Color(0.0f,0.f,0.f));
        }
        stdPal.ToContainer(palC);
        bc.SetContainer(FIRST_PALETTE,palC);
        doc->BaseList2D::GetDataInstance()->SetContainer(PALETTE_SCENE_HOOK_ID, bc);
    }
}

void Palette::GetPalettes(GeDynamicArray<Palette> &palettes)
{
	BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID);
	Int32 count = bc->GetInt32(NUM_PALETTES);
	palettes.SetCount(0);
	Palette pal;
	for(int i=0;i<count;i++){
		BaseContainer *c = bc->GetContainerInstance(FIRST_PALETTE+i);
		if(c != nullptr){
			pal.FromContainer(*c);
			palettes.Push(pal);
            palettes[i].m_name = pal.m_name;
		}
	}
}

BaseContainer *Palette::GetPaletteContainer(Int32 paletteID, BaseContainer *bc)
{
	Int32 count = bc->GetInt32(NUM_PALETTES);
	if(paletteID < count){
		return bc->GetContainerInstance(FIRST_PALETTE+paletteID);
	}
	return nullptr;
}

void Palette::SetPaletteColor(Int32 paletteID, Int32 colorID, const Color &col)
{
	BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID);
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
		Palette pal;
		pal.FromContainer(*c);
		pal.SetColor(colorID,col);
		pal.ToContainer(*c);
		bc->SetContainer(FIRST_PALETTE+paletteID,*c);
        UpdatePalette(paletteID);
        UpdateColor(paletteID,colorID);
        GetActiveDocument()->SetChanged();
	}
}

void Palette::InsertPaletteColor(Int32 paletteID, Int32 colorID, const Color &col)
{
	BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID);
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
        GeDynamicArray<Int32> newIds;
		Palette pal;
		pal.FromContainer(*c);
        for(Int32 i=0;i<pal.m_colors.GetCount();i++){
            if(i<colorID){
                newIds.Push(i);
            } else {
                newIds.Push(i+1);
            }
        }
		pal.InsertColor(colorID,col);
		pal.ToContainer(*c);
		bc->SetContainer(FIRST_PALETTE+paletteID,*c);
		UpdatePalette(paletteID);
        ReorderPalette(paletteID, &newIds);
        GetActiveDocument()->SetChanged();
	}
}


void Palette::RemovePaletteColor(Int32 paletteID, Int32 colorID)
{
    BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID);
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
        GeDynamicArray<Int32> newIds;
		Palette pal;
		pal.FromContainer(*c);
        for(Int32 i=0;i<pal.m_colors.GetCount();i++){
            if(i<colorID){
                newIds.Push(i);
            } else if(i == colorID){
                newIds.Push(0);
            } else {
                newIds.Push(i-1);
            }
        }
		pal.RemoveColor(colorID);
		pal.ToContainer(*c);
		bc->SetContainer(FIRST_PALETTE+paletteID,*c);
		UpdatePalette(paletteID);
        ReorderPalette(paletteID, &newIds);
        GetActiveDocument()->SetChanged();
	}
}

void Palette::GetPaletteColor(Int32 paletteID, Int32 colorID, Color &col)
{
	BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID);
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
		Palette pal;
		pal.FromContainer(*c);
        if(pal.m_colors.GetCount() > colorID){
            col = pal[colorID];
        }
	}
}

void Palette::SetPaletteName(Int32 paletteID, String name)
{
    BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID);
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
		Palette pal;
		pal.FromContainer(*c);
		pal.m_name = name;
		pal.ToContainer(*c);
		bc->SetContainer(FIRST_PALETTE+paletteID,*c);
        UpdatePalette(paletteID);
        GetActiveDocument()->SetChanged();
	}
}


Int32 Palette::AddPalette(const Palette &palette)
{
    BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_SCENE_HOOK_ID);
	Int32 count = bc->GetInt32(NUM_PALETTES);
	BaseContainer pal;
	palette.ToContainer(pal);
	bc->SetContainer(FIRST_PALETTE+count,pal);
	bc->SetInt32(NUM_PALETTES,count+1);
    GetActiveDocument()->SetChanged();
	return count;
}

void Palette::UpdateAll()
{
    SpecialEventAdd(PALETTE_ID,-1,-1);
}

void Palette::UpdatePalette(Int32 id)
{
	SpecialEventAdd(PALETTE_ID,-1,(Int64)id);
}

void Palette::ReorderPalette(Int32 id, GeDynamicArray<Int32> *newIDs)
{
    BaseDocument *doc = GetActiveDocument();
    BaseMaterial *mat = doc->GetFirstMaterial();
    while(mat != nullptr){
        ReorderPaletteData data;
        data.newIDs = newIDs;
        data.mat = mat;
        data.paletteID = id;
        mat->MultiMessage(MULTIMSG_ROUTE_BROADCAST, PALETTE_ID, &data);
        mat = mat->GetNext();
    }
}

void Palette::UpdateColor(Int32 palette, Int32 color)
{
    //SpecialEventAdd(PALETTE_ID,color,(Int64)palette);
    BaseDocument *doc = GetActiveDocument();
    BaseMaterial *mat = doc->GetFirstMaterial();
    while(mat != nullptr){
        ReorderPaletteData data;
        data.newIDs = nullptr;
        data.mat = mat;
        data.colorID = color;
        data.paletteID = palette;
        mat->MultiMessage(MULTIMSG_ROUTE_BROADCAST, PALETTE_ID, &data);
        mat = mat->GetNext();
    }
}

Bool Palette::LoadASEFile(String s, Palette &pal)
{
    ASE_FILE aseFile;
    Int32 fnLength =  s.GetCStringLen(STRINGENCODING_UTF8);
    char *str = NewMem(char,fnLength+1);
    s.GetCString(str, fnLength+1, STRINGENCODING_UTF8);
    ASE_ERRORTYPE error = ase_openAndReadAseFile(&aseFile, str);
    DeleteMem(str);
    if(!error){
        for(Int32 i=0;i<aseFile.numGroups;i++){
            ASE_GROUP *group = aseFile.groups + i;
            pal.m_name = String((UInt16*)group->name);
            
            for(Int32 ii=0;ii<group->numColors;ii++){
                ASE_COLOR *color = group->colors + ii;
                Int32 a = 0;
                Vector col;
                while(color->col[a] != -1.0f && a < 3){
                    col[a] = color->col[a];
                    a++;
                }
                COLOR_SOURCE colorSource = COLOR_SOURCE_DISPLAY;
                switch(color->type){
                    case ASE_COLORTYPE_LAB:
                        colorSource = COLOR_SOURCE_LAB;
                        col[0] *= 100.0;
                        break;
                    case ASE_COLORTYPE_CMYK:
                        colorSource = COLOR_SOURCE_CMYK;
                        break;
                }
                Color tmp = Color(col).SetSource(colorSource).Convert(COLOR_SOURCE_DISPLAY);
                ClampColor(tmp);
                pal.SetColor(ii, NamedColor(tmp,String((UInt16*)color->name)));
            }
        }
        ase_freeAseFile(&aseFile);
        if(pal.m_name == String("")){
            pal.m_name = Filename(s).GetFileString();
        }
    } else {
        GePrint("Could not load file " + String(str) + " " + String(ase_getErrorString(error)));
    }
    return !error;
}

void Palette::SaveASEFile(String s, const Palette &pal)
{
    ASE_FILE aseFile;
    Int32 fnLength =  s.GetCStringLen();
    char *str = NewMem(char,fnLength+1);
    s.GetCString(str, fnLength+1);
    aseFile.numGroups = 1;
    aseFile.groups = NewMem(ASE_GROUP, 1);
    ASE_GROUP *group = aseFile.groups;
    UInt16 numChar = pal.m_name.GetLength() + 1;
    UInt16 *buffer = NewMem(UInt16, numChar);
    pal.m_name.GetUcBlockNull(buffer, numChar);
    group->name = (char *)buffer;
    UInt16 numColors = pal.GetCount();
    group->numColors = numColors;
    group->colors = NewMem(ASE_COLOR, numColors);
    for(UInt16 i=0;i<numColors;++i){
        ASE_COLOR *col = group->colors + i;
        NamedColor color(pal[i]);
        Vector v = color.Convert(COLOR_SOURCE_DISPLAY).AsVector();
        numChar = color.m_name.GetLength()+1;
        col->name = (char *)NewMem(UInt16, numChar);
        color.m_name.GetUcBlockNull((UInt16 *)col->name, numChar);
        for(UInt16 ii=0;ii<3;++ii){
            col->col[ii] = v[ii];
        }
        col->col[3] = -1.0f;
        col->type = ASE_COLORTYPE_RGB;
    }
    ASE_ERRORTYPE error = ase_openAndWriteAseFile(&aseFile, str);
    for(UInt16 i=0;i<numColors;++i){
        DeleteMem(group->colors[i].name);
    }
    DeleteMem(group->colors);
    DeleteMem(aseFile.groups);
    DeleteMem(buffer);
    DeleteMem(str);
}