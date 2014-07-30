#include "palette.h"

Palette::Palette():
	m_name("Unnamed")
{
}

Palette::Palette(String name, Int32 numColors):
	m_name(name), m_colors(numColors)
{
}

Palette::Palette(const Palette& pal):
	m_name(pal.m_name)
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
		bc.SetVector(FIRST_COLOR+i,m_colors[i].Convert(COLOR_SOURCE_DISPLAY).AsVector());
	}
	bc.SetString(PALETTE_NAME,m_name);
}

void Palette::FromContainer(const BaseContainer &bc)
{
	Int32 count = bc.GetInt32(NUM_COLORS);
	m_colors.SetCount(0);
	for(int i=0;i<count;i++){
		m_colors.Push(Color(bc.GetVector(FIRST_COLOR+i)).SetSource(COLOR_SOURCE_DISPLAY));
	}
	bc.GetString(PALETTE_NAME,m_name);
}

void Palette::SetColor(Int32 index, const Vector &color, COLOR_SOURCE source)
{
	SetColor(index,Color(color).SetSource(source));
}

void Palette::SetColor(Int32 index, const Color &color)
{
	while(index >= m_colors.GetCount()){
		m_colors.Push(Color());
	}
	m_colors[index] = color;
}

void Palette::InsertColor(LONG index, const Color &color)
{
	GePrint("Insert Color!");
	if(index >= m_colors.GetCount()){
		GePrint("a");
		while(index >= m_colors.GetCount()){
			GePrint("aa");
			m_colors.Push(Color());
		}
		m_colors[index] = color;
	}
	else{
		GePrint("b");
		m_colors.Insert(color,index);
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
	SetWorldPluginData(PALETTE_ID,bc,FALSE);
}

void Palette::GetPalettes(GeDynamicArray<Palette> &palettes)
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
	Int32 count = bc->GetInt32(NUM_PALETTES);
	palettes.SetCount(0);
	Palette pal;
	for(int i=0;i<count;i++){
		BaseContainer *c = bc->GetContainerInstance(FIRST_PALETTE+i);
		if(c != nullptr){
			pal.FromContainer(*c);
			palettes.Push(pal);
		}
	}
}

<<<<<<< HEAD
Int32 Palette::SetPalette(const Palette &palette)
=======
BaseContainer *Palette::GetPaletteContainer(LONG paletteID, BaseContainer *bc)
{
	LONG count = bc->GetLong(NUM_PALETTES);
	if(paletteID < count){
		return bc->GetContainerInstance(FIRST_PALETTE+paletteID);
	}
	return nullptr;
}

/*LONG Palette::SetPalette(const Palette &palette)
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
	Int32 count = bc->GetInt32(NUM_PALETTES);
	Palette pal;
	for(int i=0;i<count;i++){
		BaseContainer *c = bc->GetContainerInstance(FIRST_PALETTE+i);
		if(c != nullptr){
			pal.FromContainer(*c);
			if(palette.m_name == pal.m_name){
				// Replace palette
				BaseContainer tmp;
				palette.ToContainer(tmp);
				bc->SetContainer(FIRST_PALETTE+i,tmp);
				SetWorldPluginData(PALETTE_ID,*bc,FALSE);
				return i;
			}
		}
	}
	// Palette not found, add it
	return AddPalette(palette);
}*/

void Palette::SetPaletteColor(Int32 paletteID, Int32 colorID, const Color &col)
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
<<<<<<< HEAD
	Int32 count = bc->GetInt32(NUM_PALETTES);
	if(paletteID < count){
		BaseContainer *c = bc->GetContainerInstance(FIRST_PALETTE+paletteID);
		if(c!= nullptr){
			Palette pal;
			pal.FromContainer(*c);
			pal.SetColor(colorID,col);
			pal.ToContainer(*c);
			bc->SetContainer(FIRST_PALETTE+paletteID,*c);
			SetWorldPluginData(PALETTE_ID,*bc,FALSE);
			UpdateColor(paletteID, colorID);
		}
=======
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
		Palette pal;
		pal.FromContainer(*c);
		pal.SetColor(colorID,col);
		pal.ToContainer(*c);
		bc->SetContainer(FIRST_PALETTE+paletteID,*c);
		SetWorldPluginData(PALETTE_ID,*bc,FALSE);
		UpdateColor(paletteID, colorID);
	}
}

void Palette::InsertPaletteColor(LONG paletteID, LONG colorID, const Color &col)
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
		Palette pal;
		pal.FromContainer(*c);
		pal.InsertColor(colorID,col);
		pal.ToContainer(*c);
		bc->SetContainer(FIRST_PALETTE+paletteID,*c);
		SetWorldPluginData(PALETTE_ID,*bc,FALSE);
		UpdatePalette(paletteID);
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
	}
}

void Palette::GetPaletteColor(Int32 paletteID, Int32 colorID, Color &col)
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
<<<<<<< HEAD
	Int32 count = bc->GetInt32(NUM_PALETTES);
	if(paletteID < count){
		BaseContainer *c = bc->GetContainerInstance(FIRST_PALETTE+paletteID);
		if(c!= nullptr){
			Palette pal;
			pal.FromContainer(*c);
			col = pal[colorID];
		}
=======
	BaseContainer *c = GetPaletteContainer(paletteID,bc);
	if(c!= nullptr){
		Palette pal;
		pal.FromContainer(*c);
		col = pal[colorID];
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
	}
}


Int32 Palette::AddPalette(const Palette &palette)
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
	Int32 count = bc->GetInt32(NUM_PALETTES);
	BaseContainer pal;
	palette.ToContainer(pal);
	bc->SetContainer(FIRST_PALETTE+count,pal);
	bc->SetInt32(NUM_PALETTES,count+1);
	SetWorldPluginData(PALETTE_ID,*bc,FALSE);
	return count;
}

void Palette::UpdatePalette(Int32 id)
{
	SpecialEventAdd(PALETTE_ID,-1,id);
}

void Palette::UpdateColor(Int32 palette, Int32 color)
{
	SpecialEventAdd(PALETTE_ID,color,palette);
}