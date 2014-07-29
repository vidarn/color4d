#include "colorbox.h"
#include "colordialog.h"

ColorBox::ColorBox(ColorDialog *parent)
{
	m_w = 200;
	m_h = 200;
	m_parent = parent;
	m_bitmap = BaseBitmap::Alloc();
	m_color.SetSource(COLOR_SOURCE_WHEEL);
}

ColorBox::~ColorBox(void)
{
	BaseBitmap::Free(m_bitmap);
}

Bool ColorBox::Init(void)
{
	UpdateCircle();
	return TRUE;
}

void ColorBox::UpdateCircle()
{
	const Float valueRadius = 7;
	const Float aaBuffer = 2;
	const Float valueSeparator = 2;
	IMAGERESULT result = m_bitmap->Init(m_w,m_h,32,INITBITMAPFLAGS_0);
	for(Int32 y=0;y<m_h;y++){
		for(Int32 x=0;x<m_w;x++){
			Vector col = Color(m_color[0],x/Float(m_w),y/Float(m_h)).SetSource(COLOR_SOURCE_WHEEL).Convert(COLOR_SOURCE_DISPLAY).AsVector();
			Int32 currX =  m_color[1]*m_w;
			Int32 currY = m_color[2]*m_h;
			Float dx = x - currX;
			Float dy = y - currY;
			Float dist = Sqrt(Float(dx*dx + dy*dy));
			if(dist < valueRadius){
				Float val = Smoothstep(valueRadius-aaBuffer,valueRadius,dist);
				col = Vector(0.0, 0.0, 0.0)*(1.0-val) + col*val;
				if(dist < valueRadius-valueSeparator){
					Float val = Smoothstep(valueRadius-valueSeparator-aaBuffer,valueRadius-valueSeparator,dist);
					col = Vector(0.7,0.7,0.7)*(1.0-val) + col*val;
				}
			}
			m_bitmap->SetPixel(x,y,255*col.x,255*col.y,255*col.z);
		}
	}
}

Bool ColorBox::GetMinSize(Int32 &w,Int32 &h)
{
	w = m_w;
	h = m_h;
	return TRUE;
}

void ColorBox::Sized(Int32 w,Int32 h)
{
	m_w = w;
	m_h = h;
	m_centerX = m_w*0.5;
	m_centerY = m_h*0.5;
	UpdateCircle();
	Redraw();
}

void ColorBox::DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg)
{
	// skip the redraw in case if focus change
	Int32 reason = msg.GetInt32(BFM_DRAW_REASON);
	if (reason==BFM_GOTFOCUS || reason==BFM_LOSTFOCUS) 
		return;
	OffScreenOn();
	DrawBitmap(m_bitmap,x1,y1,m_bitmap->GetBw(),m_bitmap->GetBh(),0,0,m_bitmap->GetBw(),m_bitmap->GetBh(),BMP_NORMAL);
}

void ColorBox::UpdateColor(Color color){
	m_color = color;
	UpdateCircle();
	Redraw();
}

void ColorBox::MouseUpdate(){
	Float saturation = ClampValue(m_mouseX/Float(m_w),0.0,1.0);
	m_color[1] = saturation;
	Float lightness = ClampValue(m_mouseY/Float(m_h),0.0,1.0);
	m_color[2] = lightness;
	m_parent->UpdateColor(m_color);
}

Bool ColorBox::InputEvent(const BaseContainer &msg)
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
		//Global2Local(&m_mouseX, &m_mouseY);
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


void ColorBox::SetColor(Color color){
	m_color = color;
}

Color ColorBox::GetColor(){
	return m_color;
}
