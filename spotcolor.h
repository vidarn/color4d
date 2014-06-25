#pragma once
#include "c4d.h"
#include "color.h"
#include "lib_clipmap.h"

class ColorDialog;

class SpotColor : public GeUserArea
{
	public:

		SpotColor();
		SpotColor(ColorDialog *parent);
		~SpotColor(void);

		virtual Bool Init(void);
		virtual Bool GetMinSize(LONG &w,LONG &h);
		virtual void Sized(LONG w,LONG h);
		virtual void DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg);
		virtual Bool InputEvent(const BaseContainer &msg);

		void SetParent(ColorDialog *parent);
		virtual void UpdateColor(Color color);
		void MouseUpdate();
		void SetColor(Color color);
		Color GetColor();

		ColorDialog *m_parent;
		Color m_color;
		
		LONG m_w;
		LONG m_h;
};
