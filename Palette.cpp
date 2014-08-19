#include "palette.h"

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
	SetColor(index, NamedColor(color,""));
}

void Palette::SetColor(Int32 index, const NamedColor &color)
{
	while(index >= m_colors.GetCount()){
		m_colors.Push(NamedColor());
	}
	m_colors[index] = color;
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
    if(doc->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_ID) == nullptr){
        GePrint("Added default palette");
        BaseContainer bc;
        bc.SetInt32(NUM_PALETTES,1);
        BaseContainer palC;
        Palette stdPal(String("Default"),5);
        stdPal.SetColor(0,Color(1.0f,0.f,0.f));
        stdPal.SetColor(1,Color(1.0f,1.f,0.f));
        stdPal.SetColor(2,Color(1.0f,1.f,1.f));
        stdPal.SetColor(3,Color(0.0f,1.f,1.f));
        stdPal.SetColor(4,Color(0.0f,0.f,1.f));
        stdPal.ToContainer(palC);
        bc.SetContainer(FIRST_PALETTE,palC);
        doc->BaseList2D::GetDataInstance()->SetContainer(PALETTE_ID, bc);
    }
}

void Palette::GetPalettes(GeDynamicArray<Palette> &palettes)
{
	BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_ID);
	Int32 count = bc->GetInt32(NUM_PALETTES);
	palettes.SetCount(0);
	Palette pal;
	for(int i=0;i<count;i++){
		BaseContainer *c = bc->GetContainerInstance(FIRST_PALETTE+i);
		if(c != nullptr){
			pal.FromContainer(*c);
			palettes.Push(pal);
            palettes[palettes.GetCount()-1].m_name = pal.m_name;
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
	BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_ID);
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
		Palette pal;
		pal.FromContainer(*c);
		pal.SetColor(colorID,col);
		pal.ToContainer(*c);
		bc->SetContainer(FIRST_PALETTE+paletteID,*c);
		SetWorldPluginData(PALETTE_ID,*bc,FALSE);
		//UpdateColor(paletteID, colorID);
        UpdatePalette(paletteID);
	}
}

void Palette::InsertPaletteColor(Int32 paletteID, Int32 colorID, const Color &col)
{
	BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_ID);
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
		Palette pal;
		pal.FromContainer(*c);
		pal.InsertColor(colorID,col);
		pal.ToContainer(*c);
		bc->SetContainer(FIRST_PALETTE+paletteID,*c);
		SetWorldPluginData(PALETTE_ID,*bc,FALSE);
		UpdatePalette(paletteID);
	}
}

void Palette::GetPaletteColor(Int32 paletteID, Int32 colorID, Color &col)
{
	BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_ID);
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
		Palette pal;
		pal.FromContainer(*c);
		col = pal[colorID];
	}
}


Int32 Palette::AddPalette(const Palette &palette)
{
    BaseContainer *bc = GetActiveDocument()->BaseList2D::GetDataInstance()->GetContainerInstance(PALETTE_ID);
	Int32 count = bc->GetInt32(NUM_PALETTES);
	BaseContainer pal;
	palette.ToContainer(pal);
	bc->SetContainer(FIRST_PALETTE+count,pal);
	bc->SetInt32(NUM_PALETTES,count+1);
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

void Palette::UpdateColor(Int32 palette, Int32 color)
{
	SpecialEventAdd(PALETTE_ID,color,(Int64)palette);
}