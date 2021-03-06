#pragma once
#include "namedcolor.h"
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

struct ReorderPaletteData
{
    Int32 paletteID, colorID;
    GeDynamicArray<Int32> *newIDs;
    BaseMaterial *mat;
};

class Palette
{
	public:
		Palette();
		Palette(String name, Int32 numColors=3);
		Palette(const Palette& pal);
		void ToContainer(BaseContainer &bc) const;
		void FromContainer(const BaseContainer &bc);
		void SetColor(Int32 index, const Vector &color, COLOR_SOURCE source);
		void SetColor(Int32 index, const Color &color);
        void SetColor(Int32 index, const NamedColor &color);
		void InsertColor(Int32 index, const Color &color);
        void RemoveColor(Int32 index);
		Int32 GetCount() const {return  m_colors.GetCount();}
		NamedColor & operator[](int i){return m_colors[i];}
        const NamedColor & operator[](int i) const {return m_colors[i];}
		const Palette &operator=(const Palette &pal);
    
        String m_name;
        Bool m_inScene;

		static void InitPalettes();
		static void GetPalettes(GeDynamicArray<Palette> &palettes);
		static BaseContainer *GetPaletteContainer(Int32 paletteID,BaseContainer *bc);
		//static Int32 SetPalette(const Palette &palette);
		static void SetPaletteColor(Int32 paletteID, Int32 colorID, const Color &col);
		static void InsertPaletteColor(Int32 paletteID, Int32 colorID, const Color &col);
        static void RemovePaletteColor(Int32 paletteID, Int32 colorID);
		static void GetPaletteColor(Int32 paletteID, Int32 colorID, Color &col);
        static void SetPaletteName(Int32 paletteID, String name);
		static Int32 AddPalette(const Palette &palette);
        static void UpdateAll();
		static void UpdatePalette(Int32 id);
        static void ReorderPalette(Int32 id, GeDynamicArray<Int32> *newIDs);
		static void UpdateColor(Int32 palette, Int32 color);
        static Bool LoadASEFile(String filename, Palette &palette);
        static void SaveASEFile(String filename, const Palette &palette);
    
		GeDynamicArray<NamedColor> m_colors;
};