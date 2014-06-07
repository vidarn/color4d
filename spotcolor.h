#pragma once
#include "c4d.h"

class ColorPickerDialog;

class SpotColor : public GeUserArea
{
	public:

		SpotColor();
		SpotColor(ColorPickerDialog *parent);
		~SpotColor(void);

		virtual Bool Init(void);
		virtual Bool GetMinSize(LONG &w,LONG &h);
		virtual void Sized(LONG w,LONG h);
		virtual void DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg);
		virtual Bool InputEvent(const BaseContainer &msg);

		void SetParent(ColorPickerDialog *parent);
		void UpdateColor(Vector color);
		void MouseUpdate();
		void SetColor(Vector color);
		Vector GetColor();

		ColorPickerDialog *m_parent;
		Vector m_color;
		LONG m_w;
		LONG m_h;
};
