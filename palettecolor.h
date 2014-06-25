#pragma once

#include "spotcolor.h"

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
		PaletteColor(ColorDialog *parent);
		PaletteColor();
		~PaletteColor();

		virtual Bool Init(void);
		virtual void Sized(LONG w,LONG h);
		virtual void DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg);
		virtual LONG Message(const BaseContainer& msg, BaseContainer& result);
		virtual void UpdateColor(Color color);
		void UpdateBitmaps();
		static void LoadIcons();
		static void UnloadIcons();

	private:
		HOVER_STATE m_hoverState;
		static BaseBitmap *m_refreshIcon;
		static BaseBitmap *m_leftArrowIcon;
		static BaseBitmap *m_rightArrowIcon;
		BaseBitmap *m_normalBitmap;
		BaseBitmap *m_hoverBitmap;
		BaseBitmap *m_leftHoverBitmap;
		BaseBitmap *m_rightHoverBitmap;
};