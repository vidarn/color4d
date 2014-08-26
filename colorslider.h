#pragma once
#include "c4d.h"
#include "c4d_basebitmap.h"
#include "color.h"

class ColorDialog;

class ColorSlider : public GeUserArea
{
	public:

		ColorSlider(ColorDialog *parent, Int32 index, COLOR_SOURCE colorSource);
		ColorSlider();
		~ColorSlider(void);

		virtual Bool Init(void);
		virtual Bool GetMinSize(Int32 &w,Int32 &h);
		virtual void Sized(Int32 w,Int32 h);
		virtual void DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg);
		virtual Bool InputEvent(const BaseContainer &msg);

		void UpdateColorWithValue(Float value, Color &color);
		void UpdateColor(Color color);
		void UpdateCircle();
		void MouseUpdate();
		void SetColor(Color color);
		void SetParent(ColorDialog *parent);
		void SetIndex(Int32 index);
		void SetColorSource(COLOR_SOURCE colorSource);
		void SetValueMax(Float max);
		Color GetColor();

		COLOR_SOURCE m_colorSource;
		ColorDialog *m_parent;
		BaseBitmap *m_bitmap;
		Int32 m_mouseX;
		Int32 m_mouseY;
		Int32 m_index;
		Float m_value;
		Float m_valueMax;
        Float m_valueMin;
		Color m_color;
		Int32 m_w;
		Int32 m_h;
		Float m_centerX;
		Float m_centerY;
		Bool m_mouseDown;
};