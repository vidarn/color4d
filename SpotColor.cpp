#include "spotcolor.h"
#include "colordialog.h"



SpotColor::SpotColor(ColorDialog *parent)
{
	SpotColor();
	m_parent = parent;
}

SpotColor::SpotColor()
{
	m_w = 40;
	m_h = 40;
	m_color = Color(0.32,1.0,0.5);
	m_parent = NULL;
}

SpotColor::~SpotColor(void)
{
}

void SpotColor::SetParent(ColorDialog *parent)
{
	m_parent = parent;
}

Bool SpotColor::Init(void)
{
	GePrint("Init!");
	return TRUE;
}

Bool SpotColor::GetMinSize(LONG &w,LONG &h)
{
	w = 40;
	h = 40;
	return TRUE;
}

void SpotColor::Sized(LONG w,LONG h)
{
	m_w = w;
	m_h = h;
	Redraw();
}

void SpotColor::DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg)
{
	OffScreenOn();
	DrawSetPen(m_color.Convert(COLOR_SOURCE_DISPLAY).AsVector());
	DrawRectangle(x1,y1,x2,y2);
}

void SpotColor::UpdateColor(Color color){
	SetColor(color);
	Redraw();
}

Bool SpotColor::InputEvent(const BaseContainer &msg)
{
	if(msg.GetLong(BFM_INPUT_DEVICE) == BFM_INPUT_MOUSE){
		if(msg.GetLong(BFM_INPUT_CHANNEL) == BFM_INPUT_MOUSELEFT){
			Vector col = m_color.Convert(COLOR_SOURCE_DISPLAY).AsVector();
			HandleMouseDrag(msg,DRAGTYPE_RGB,&col,0);
		}
	}
	return FALSE;
}

void SpotColor::SetColor(Color color){
	m_color = color;
}

Color SpotColor::GetColor(){
	return m_color;
}
