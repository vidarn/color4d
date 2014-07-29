#pragma once

#include "spotcolor.h"
#include "palette.h"

enum HOVER_STATE
{
	HOVER_NONE,
	HOVER_LEFT,
	HOVER_CENTER,
	HOVER_RIGHT,
};

class PaletteColor :public SpotColor
{
	public:
		PaletteColor();
		~PaletteColor();

		virtual Bool Init(void);
		virtual void Sized(Int32 w,Int32 h);
		virtual void DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg);
		virtual Int32 Message(const BaseContainer& msg, BaseContainer& result);
		virtual Bool CoreMessage(Int32 id, const BaseContainer& msg);
		virtual void UpdateColor(Color color);
		void UpdateBitmaps();
		void SetColorID(Int32 id){m_colorID = id;}
		void SetPaletteID(Int32 id){m_palette = id;}
		static void LoadIcons();
		static void UnloadIcons();
	private:
		Int32 m_palette, m_colorID;
		HOVER_STATE m_hoverState;
		static BaseBitmap *m_refreshIcon;
		static BaseBitmap *m_leftArrowIcon;
		static BaseBitmap *m_rightArrowIcon;
		BaseBitmap *m_normalBitmap;
		BaseBitmap *m_hoverBitmap;
		BaseBitmap *m_leftHoverBitmap;
		BaseBitmap *m_rightHoverBitmap;
};