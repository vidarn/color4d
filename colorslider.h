#pragma once
#include "c4d.h"
#include "c4d_basebitmap.h"
#include "color.h"

class ColorPickerDialog;

class ColorSlider : public GeUserArea
{
	public:

		ColorSlider(ColorPickerDialog *parent, LONG index, COLOR_SOURCE colorSource);
		ColorSlider();
		~ColorSlider(void);

		virtual Bool Init(void);
		virtual Bool GetMinSize(LONG &w,LONG &h);
		virtual void Sized(LONG w,LONG h);
		virtual void DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg);
		virtual Bool InputEvent(const BaseContainer &msg);

		void UpdateColorWithValue(Real value, Color &color);
		void UpdateColor(Color color);
		void UpdateCircle();
		void MouseUpdate();
		void SetColor(Color color);
		void SetParent(ColorPickerDialog *parent);
		void SetIndex(LONG index);
		void SetColorSource(COLOR_SOURCE colorSource);
		void SetValueMax(Real max);
		Color GetColor();

		COLOR_SOURCE m_colorSource;
		ColorPickerDialog *m_parent;
		BaseBitmap *m_bitmap;
		LONG m_mouseX;
		LONG m_mouseY;
		LONG m_index;
		Real m_value;
		Real m_valueMax;
		Color m_color;
		LONG m_w;
		LONG m_h;
		Real m_centerX;
		Real m_centerY;
		Bool m_mouseDown;
};