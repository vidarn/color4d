#pragma once
#include "c4d.h"
#include "c4d_basebitmap.h"

class ColorPickerDialog;

class ColorSlider : public GeUserArea
{
	public:

		ColorSlider(ColorPickerDialog *parent, LONG index);
		~ColorSlider(void);

		virtual Bool Init(void);
		virtual Bool GetMinSize(LONG &w,LONG &h);
		virtual void Sized(LONG w,LONG h);
		virtual void DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg);
		virtual Bool InputEvent(const BaseContainer &msg);

		void UpdateColorWithValue(Real value, Vector &color);
		void UpdateColor(Vector color);
		void UpdateCircle();
		void MouseUpdate();
		void SetColor(Vector color);
		Vector GetColor();

		ColorPickerDialog *m_parent;
		BaseBitmap *m_bitmap;
		LONG m_mouseX;
		LONG m_mouseY;
		LONG m_index;
		Real m_value;
		Vector m_color;
		LONG m_w;
		LONG m_h;
		Real m_centerX;
		Real m_centerY;
		Bool m_mouseDown;
};