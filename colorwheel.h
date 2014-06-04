#pragma once
#include "c4d.h"
#include "c4d_basebitmap.h"

class ColorPickerDialog;

class ColorWheel : public GeUserArea
{
	public:

		ColorWheel(ColorPickerDialog *parent);
		~ColorWheel(void);

		virtual Bool Init(void);
		virtual Bool GetMinSize(LONG &w,LONG &h);
		virtual void Sized(LONG w,LONG h);
		virtual void DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg);
		virtual Bool InputEvent(const BaseContainer &msg);

		void UpdateColor(Vector color);
		void UpdateCircle();
		void MouseUpdate();
		void SetColor(Vector color);
		Vector GetColor();

		ColorPickerDialog *m_parent;
		BaseBitmap *m_bitmap;
		UCHAR *m_pixels;
		LONG m_mouseX;
		LONG m_mouseY;
		Vector m_color;
		LONG m_w;
		LONG m_h;
		Real m_centerX;
		Real m_centerY;
		Bool m_mouseDown;
};