#include "palette.h"

Palette::Palette():
	m_name("Unnamed")
{
}

Palette::Palette(String name, LONG numColors):
	m_name(name), m_colors(numColors)
{
}

Palette::Palette(const Palette& pal):
	m_name(pal.m_name)
{
	LONG count = pal.m_colors.GetCount();
	m_colors.SetCount(0);
	for(int i=0;i<count;i++){
		m_colors.Push(pal.m_colors[i]);
	}
}

void Palette::ToContainer(BaseContainer &bc) const
{
	LONG count = m_colors.GetCount();
	bc.SetLong(NUM_COLORS,m_colors.GetCount());
	for(int i=0;i<count;i++){
		bc.SetVector(FIRST_COLOR+i,m_colors[i].Convert(COLOR_SOURCE_DISPLAY).AsVector());
	}
	bc.SetString(PALETTE_NAME,m_name);
}

void Palette::FromContainer(const BaseContainer &bc)
{
	LONG count = bc.GetLong(NUM_COLORS);
	m_colors.SetCount(0);
	for(int i=0;i<count;i++){
		m_colors.Push(Color(bc.GetVector(FIRST_COLOR+i)).SetSource(COLOR_SOURCE_DISPLAY));
	}
	bc.GetString(PALETTE_NAME,m_name);
}

void Palette::SetColor(LONG index, const Vector &color, COLOR_SOURCE source)
{
	SetColor(index,Color(color).SetSource(source));
}

void Palette::SetColor(LONG index, const Color &color)
{
	while(index >= m_colors.GetCount()){
		m_colors.Push(Color());
	}
	m_colors[index] = color;
}

const Palette &Palette::operator=(const Palette &pal)
{
	m_colors.SetCount(0);
	LONG count = pal.m_colors.GetCount();
	for(int i=0;i<count;i++){
		m_colors.Push(pal.m_colors[i]);
	}
	return *this;
}

//--------------------------

void Palette::InitPalettes()
{
	BaseContainer bc;
	bc.SetLong(NUM_PALETTES,1);
	BaseContainer palC;
	Palette stdPal(String("Default"),3);
	stdPal.SetColor(0,Color(1.0f,0.f,0.f));
	stdPal.SetColor(1,Color(1.0f,1.f,0.f));
	stdPal.SetColor(2,Color(1.0f,1.f,1.f));
	stdPal.ToContainer(palC);
	bc.SetContainer(FIRST_PALETTE,palC);
	SetWorldPluginData(PALETTE_ID,bc,FALSE);
}

void Palette::GetPalettes(GeDynamicArray<Palette> &palettes)
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
	LONG count = bc->GetLong(NUM_PALETTES);
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

LONG Palette::SetPalette(const Palette &palette)
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
	LONG count = bc->GetLong(NUM_PALETTES);
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
}

void Palette::SetPaletteColor(LONG paletteID, LONG colorID, const Color &col)
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
	LONG count = bc->GetLong(NUM_PALETTES);
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
	}
}

void Palette::GetPaletteColor(LONG paletteID, LONG colorID, Color &col)
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
	LONG count = bc->GetLong(NUM_PALETTES);
	if(paletteID < count){
		BaseContainer *c = bc->GetContainerInstance(FIRST_PALETTE+paletteID);
		if(c!= nullptr){
			Palette pal;
			pal.FromContainer(*c);
			col = pal[colorID];
		}
	}
}


LONG Palette::AddPalette(const Palette &palette)
{
	BaseContainer *bc = GetWorldPluginData(PALETTE_ID);
	LONG count = bc->GetLong(NUM_PALETTES);
	BaseContainer pal;
	palette.ToContainer(pal);
	bc->SetContainer(FIRST_PALETTE+count,pal);
	bc->SetLong(NUM_PALETTES,count+1);
	SetWorldPluginData(PALETTE_ID,*bc,FALSE);
	return count;
}

void Palette::UpdatePalette(LONG id)
{
	SpecialEventAdd(PALETTE_ID,-1,id);
}

void Palette::UpdateColor(LONG palette, LONG color)
{
	SpecialEventAdd(PALETTE_ID,color,palette);
}