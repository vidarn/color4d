#include "spotcolor.h"
#include "colordialog.h"
#include "utils.h"


SpotColor::SpotColor(ColorDialog *parent)
{
	SpotColor();
	m_parent = parent;
}

SpotColor::SpotColor():
m_dragable(TRUE)
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
	return TRUE;
}

Bool SpotColor::GetMinSize(Int32 &w,Int32 &h)
{
	w = 40;
	h = 40;
	return TRUE;
}

void SpotColor::Sized(Int32 w,Int32 h)
{
	m_w = w;
	m_h = h;
	Redraw();
}

void SpotColor::DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg)
{
	OffScreenOn();
    Vector tmp = m_color.Convert(COLOR_SOURCE_DISPLAY).AsVector();
    ClampColor(tmp);
	DrawSetPen(tmp);
	DrawRectangle(x1,y1,x2,y2);
}

void SpotColor::UpdateColor(Color color){
	SetColor(color);
	Redraw();
}

Bool SpotColor::InputEvent(const BaseContainer &msg)
{
	if(msg.GetInt32(BFM_INPUT_DEVICE) == BFM_INPUT_MOUSE){
		if(msg.GetInt32(BFM_INPUT_CHANNEL) == BFM_INPUT_MOUSELEFT){
			if(m_dragable){
				Vector col = m_color.Convert(COLOR_SOURCE_DISPLAY).AsVector();
                ClampColor(col);
				if(!HandleMouseDrag(msg,DRAGTYPE_RGB,&col,0)){
                    HandleClick();
                }
			}
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

void SpotColor::HandleClick()
{
}
