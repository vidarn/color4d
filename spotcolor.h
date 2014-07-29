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
		virtual Bool GetMinSize(Int32 &w,Int32 &h);
		virtual void Sized(Int32 w,Int32 h);
		virtual void DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg);
		virtual Bool InputEvent(const BaseContainer &msg);

		void SetParent(ColorDialog *parent);
		virtual void UpdateColor(Color color);
		void MouseUpdate();
		void SetColor(Color color);
		Color GetColor();

		ColorDialog *m_parent;
		Color m_color;
		
		Int32 m_w;
		Int32 m_h;
};
