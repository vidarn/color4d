#pragma once
#include "color.h"
#include "ge_dynamicarray.h"

enum {
	NUM_COLORS = 10371,
	PALETTE_NAME,
	FIRST_COLOR
};
enum {
	NUM_PALETTES = 284,
	FIRST_PALETTE
};

class Palette
{
	public:
		Palette();
		Palette(String name, LONG numColors=3);
		Palette(const Palette& pal);
		void ToContainer(BaseContainer &bc) const;
		void FromContainer(const BaseContainer &bc);
		void SetColor(LONG index, const Vector &color, COLOR_SOURCE source);
		void SetColor(LONG index, const Color &color);
		LONG GetCount(){return  m_colors.GetCount();}
		Color & operator[](int i){return m_colors[i];}
		const Palette &operator=(const Palette &pal);

		static void InitPalettes();
		static void GetPalettes(GeDynamicArray<Palette> &palettes);
		static LONG SetPalette(const Palette &palette);
		static void SetPaletteColor(LONG paletteID, LONG colorID, const Color &col);
		static void GetPaletteColor(LONG paletteID, LONG colorID, Color &col);
		static LONG AddPalette(const Palette &palette);
		static void UpdatePalette(LONG id);
		static void UpdateColor(LONG palette, LONG color);
	private:
		String m_name;
		GeDynamicArray<Color> m_colors;
};