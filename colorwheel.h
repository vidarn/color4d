#pragma once
#include "c4d.h"
#include "c4d_basebitmap.h"
#include "color.h"
#include "ge_dynamicarray.h"
#include "lib_clipmap.h"

class ColorDialog;

class ColorWheel : public GeUserArea
{
	public:

		ColorWheel(ColorDialog *parent);
		~ColorWheel(void);

		virtual Bool Init(void);
		virtual Bool GetMinSize(Int32 &w,Int32 &h);
		virtual void Sized(Int32 w,Int32 h);
		virtual void DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg);
		virtual Bool InputEvent(const BaseContainer &msg);

		void UpdateColor(Color color);
		void UpdateCircle();
		void UpdateMarker();
		void UpdateCanvas();
		void MouseUpdate();
		void SetOffsets(const GeDynamicArray<Float> &offsets);
		void GetOffsetColors(GeDynamicArray<Color> &colors);
		void SetColor(Color color);
		Color GetColor();

		ColorDialog *m_parent;
		GeDynamicArray<Float> m_offsets;
		GeClipMap *m_wheelClipMap;
		GeClipMap *m_markerClipMap;
		GeClipMap *m_canvas;
		Int32 m_mouseX;
		Int32 m_mouseY;
		Color m_color;
		Int32 m_w;
		Int32 m_h;
		Float m_centerX;
		Float m_centerY;
		Bool m_mouseDown;
		Int32 m_selectedMarker;

		const Float m_valueRadius;
		const Float m_valuePosition;
};