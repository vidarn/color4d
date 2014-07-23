#pragma once
#include "c4d.h"
#include "c4d_basebitmap.h"
#include "color.h"
#include "ge_dynamicarray.h"
#include "lib_clipmap.h"
#include "colorscheme.h"

class ColorDialog;

class ColorWheel : public GeUserArea
{
	public:

		ColorWheel(ColorDialog *parent);
		~ColorWheel(void);

		virtual Bool Init(void);
		virtual Bool GetMinSize(LONG &w,LONG &h);
		virtual void Sized(LONG w,LONG h);
		virtual void DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg);
		virtual Bool InputEvent(const BaseContainer &msg);

		void UpdateColor(Color color);
		void UpdateCircle();
		void UpdateMarker();
		void UpdateCanvas();
		void MouseUpdate();
		void SetOffsets(const GeDynamicArray<Real> &offsets);
		void GetOffsetColors(GeDynamicArray<Color> &colors);
		void SetColor(Color color);
		void SetScheme(ColorScheme *scheme);
		Color GetColor();

		ColorDialog *m_parent;
		GeDynamicArray<Real> m_offsets;
		GeClipMap *m_wheelClipMap;
		GeClipMap *m_markerClipMap;
		GeClipMap *m_canvas;
		ColorScheme *m_scheme;
		LONG m_mouseX;
		LONG m_mouseY;
		Color m_color;
		LONG m_w;
		LONG m_h;
		Real m_centerX;
		Real m_centerY;
		Bool m_mouseDown;
		LONG m_selectedMarker;

		const Real m_valueRadius;
		const Real m_valuePosition;
};