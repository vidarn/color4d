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
		Palette(String name, Int32 numColors=3);
		Palette(const Palette& pal);
		void ToContainer(BaseContainer &bc) const;
		void FromContainer(const BaseContainer &bc);
<<<<<<< HEAD
		void SetColor(Int32 index, const Vector &color, COLOR_SOURCE source);
		void SetColor(Int32 index, const Color &color);
		Int32 GetCount(){return  m_colors.GetCount();}
=======
		void SetColor(LONG index, const Vector &color, COLOR_SOURCE source);
		void SetColor(LONG index, const Color &color);
		void InsertColor(LONG index, const Color &color);
		LONG GetCount(){return  m_colors.GetCount();}
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
		Color & operator[](int i){return m_colors[i];}
		const Palette &operator=(const Palette &pal);

		static void InitPalettes();
		static void GetPalettes(GeDynamicArray<Palette> &palettes);
<<<<<<< HEAD
		static Int32 SetPalette(const Palette &palette);
		static void SetPaletteColor(Int32 paletteID, Int32 colorID, const Color &col);
		static void GetPaletteColor(Int32 paletteID, Int32 colorID, Color &col);
		static Int32 AddPalette(const Palette &palette);
		static void UpdatePalette(Int32 id);
		static void UpdateColor(Int32 palette, Int32 color);
=======
		static BaseContainer *GetPaletteContainer(LONG paletteID,BaseContainer *bc);
		//static LONG SetPalette(const Palette &palette);
		static void SetPaletteColor(LONG paletteID, LONG colorID, const Color &col);
		static void InsertPaletteColor(LONG paletteID, LONG colorID, const Color &col);
		static void GetPaletteColor(LONG paletteID, LONG colorID, Color &col);
		static LONG AddPalette(const Palette &palette);
		static void UpdatePalette(LONG id);
		static void UpdateColor(LONG palette, LONG color);
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
	private:
		String m_name;
		GeDynamicArray<Color> m_colors;
};