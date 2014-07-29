#include "colorslider.h"
#include "colordialog.h"
#include "utils.h"

ColorSlider::ColorSlider(ColorDialog *parent, Int32 index, COLOR_SOURCE colorSource)
{
	ColorSlider();
	m_parent = parent;
	m_index = index;
	m_colorSource = colorSource;
	m_color.SetSource(m_colorSource);
}

ColorSlider::ColorSlider()
{
	m_w = 100;
	m_h = 15;
	m_parent = NULL;
	m_value = 0.0;
	m_index = 0;
	m_bitmap = BaseBitmap::Alloc();
	m_valueMax = 1.0;
}

ColorSlider::~ColorSlider(void)
{
	BaseBitmap::Free(m_bitmap);
}

Bool ColorSlider::Init(void)
{
	UpdateCircle();
	return TRUE;
}

void ColorSlider::UpdateCircle()
{
	if(m_parent != NULL){
		const Float valueRadius = 7;
		const Float aaBuffer = 2;
		const Float valueSeparator = 2;
		IMAGERESULT result = m_bitmap->Init(m_w,m_h,32,INITBITMAPFLAGS_0);
		for(Int32 y=0;y<m_h;y++){
			for(Int32 x=0;x<m_w;x++){
				Color tmp = m_color;
				UpdateColorWithValue(x/Float(m_w)*m_valueMax,tmp);
				Vector col = tmp.Convert(COLOR_SOURCE_DISPLAY).AsVector();
				ClampColor(col);
				Int32 currX =  m_value*m_w/m_valueMax;
				Int32 currY = m_h/2;
				Float dx = x - currX;
				Float dy = y - currY;
				Float dist = Sqrt(Float(dx*dx + dy*dy));
				if(dist < valueRadius){
					Float val = Smoothstep(valueRadius-aaBuffer,valueRadius,dist);
					col = Vector(0.0, 0.0, 0.0)*(1.0-val) + col*val;
					if(dist < valueRadius-valueSeparator){
						val = Smoothstep(valueRadius-valueSeparator-aaBuffer,valueRadius-valueSeparator,dist);
						col = Vector(0.7,0.7,0.7)*(1.0-val) + col*val;
					}
				}
				else{
					
				}
				m_bitmap->SetPixel(x,y,255*col.x,255*col.y,255*col.z);
			}
		}
	}
}

Bool ColorSlider::GetMinSize(Int32 &w,Int32 &h)
{
	w = m_w;
	h = m_h;
	return TRUE;
}

void ColorSlider::Sized(Int32 w,Int32 h)
{
	m_w = w;
	m_h = h;
	m_centerX = m_w*0.5;
	m_centerY = m_h*0.5;
	UpdateCircle();
	Redraw();
}

void ColorSlider::DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg)
{
	// skip the redraw in case if focus change
	Int32 reason = msg.GetInt32(BFM_DRAW_REASON);
	if (reason==BFM_GOTFOCUS || reason==BFM_LOSTFOCUS) 
		return;
	OffScreenOn();
	DrawBitmap(m_bitmap,x1,y1,m_bitmap->GetBw(),m_bitmap->GetBh(),0,0,m_bitmap->GetBw(),m_bitmap->GetBh(),BMP_NORMAL);
}

void ColorSlider::UpdateColor(Color color){
	SetColor(color);
	UpdateCircle();
	Redraw();
}

void ColorSlider::UpdateColorWithValue(Float value, Color &color){
	color[m_index] = value;
}

void ColorSlider::MouseUpdate(){
	m_value = ClampValue(m_mouseX/Float(m_w),0.0,1.0)*m_valueMax;
	UpdateColorWithValue(m_value,m_color);
	if(m_parent != NULL){
		m_parent->UpdateColor(m_color);
	}
}

Bool ColorSlider::InputEvent(const BaseContainer &msg)
{
	if(msg.GetInt32(BFM_INPUT_DEVICE) == BFM_INPUT_MOUSE){
		if(msg.GetInt32(BFM_INPUT_CHANNEL) == BFM_INPUT_MOUSELEFT){
			m_mouseX = msg.GetInt32(BFM_INPUT_X);
			m_mouseY = msg.GetInt32(BFM_INPUT_Y);
			m_mouseDown = TRUE;
			Global2Local(&m_mouseX, &m_mouseY);
			MouseDragStart(BFM_INPUT_MOUSELEFT,m_mouseX, m_mouseY,MOUSEDRAGFLAGS_0);
			MouseUpdate();
		}
	}
	Float x, y;
	BaseContainer channels;
	while (MouseDrag(&x, &y, &channels) == MOUSEDRAGRESULT_CONTINUE)
	{
		m_mouseX -= x;
		m_mouseY -= y;
		MouseUpdate();
	}
	BaseContainer res;
	if(GetInputState(BFM_INPUT_MOUSE,BFM_INPUT_MOUSELEFT,res) && res.GetInt32(BFM_INPUT_VALUE) == 0){
		if(m_mouseDown){
			MouseDragEnd();
			m_mouseDown = FALSE;
		}
	}

	return FALSE;
}


void ColorSlider::SetColor(Color color){
	m_color = color;
	m_value = color[m_index];
}

void ColorSlider::SetParent(ColorDialog *parent)
{
	m_parent = parent;
}

void ColorSlider::SetColorSource(COLOR_SOURCE colorSource)
{
	m_colorSource = colorSource;
	m_color.SetSource(m_colorSource);
}

void ColorSlider::SetIndex(Int32 index)
{
	m_index = index;
}

void ColorSlider::SetValueMax(Float max)
{
	m_valueMax = max;
}

Color ColorSlider::GetColor(){
	return m_color;
}
