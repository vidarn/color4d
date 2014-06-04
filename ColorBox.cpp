#include "colorbox.h"
#include "colorpickerdialog.h"

ColorBox::ColorBox(ColorPickerDialog *parent)
{
	m_w = 200;
	m_h = 200;
	m_parent = parent;
}

ColorBox::~ColorBox(void)
{
	BaseBitmap::Free(m_bitmap);
}

Bool ColorBox::Init(void)
{
	m_color = (0.32,1.0,0.5);
	m_bitmap = BaseBitmap::Alloc();
	UpdateCircle();
	return TRUE;
}

void ColorBox::UpdateCircle()
{
	const Real valueRadius = 7;
	const Real aaBuffer = 2;
	const Real valueSeparator = 2;
	IMAGERESULT result = m_bitmap->Init(m_w,m_h,32,INITBITMAPFLAGS_0);
	for(LONG y=0;y<m_h;y++){
		for(LONG x=0;x<m_w;x++){
			Vector col = Vector(m_color.x,x/Real(m_w),y/Real(m_h));
			col = HSLtoRGB(col);
			LONG currX =  m_color.y*m_w;
			LONG currY = m_color.z*m_h;
			Real dx = x - currX;
			Real dy = y - currY;
			Real dist = Sqrt(Real(dx*dx + dy*dy));
			if(dist < valueRadius){
				Real val = Smoothstep(valueRadius-aaBuffer,valueRadius,dist);
				col = Vector(0.0, 0.0, 0.0)*(1.0-val) + col*val;
				if(dist < valueRadius-valueSeparator){
					Real val = Smoothstep(valueRadius-valueSeparator-aaBuffer,valueRadius-valueSeparator,dist);
					col = Vector(0.7,0.7,0.7)*(1.0-val) + col*val;
				}
			}
			m_bitmap->SetPixel(x,y,255*col.x,255*col.y,255*col.z);
		}
	}
}

Bool ColorBox::GetMinSize(LONG &w,LONG &h)
{
	w = m_w;
	h = m_h;
	return TRUE;
}

void ColorBox::Sized(LONG w,LONG h)
{
	m_w = w;
	m_h = h;
	m_centerX = m_w*0.5;
	m_centerY = m_h*0.5;
	UpdateCircle();
	Redraw();
}

void ColorBox::DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg)
{
	// skip the redraw in case if focus change
	LONG reason = msg.GetLong(BFM_DRAW_REASON);
	if (reason==BFM_GOTFOCUS || reason==BFM_LOSTFOCUS) 
		return;
	OffScreenOn();
	DrawBitmap(m_bitmap,x1,y1,m_bitmap->GetBw(),m_bitmap->GetBh(),0,0,m_bitmap->GetBw(),m_bitmap->GetBh(),BMP_NORMAL);
}

void ColorBox::UpdateColor(Vector color){
	m_color = color;
	UpdateCircle();
	Redraw();
}

void ColorBox::MouseUpdate(){
	Real saturation = Clamp(0.0,1.0,m_mouseX/Real(m_w));
	m_color.y = saturation;
	Real lightness = Clamp(0.0,1.0,m_mouseY/Real(m_h));
	m_color.z = lightness;
	//GePrint("Saturation: " + RealToString(saturation) + " Lightness: " + RealToString(lightness));
	m_parent->UpdateColor(m_color);
}

Bool ColorBox::InputEvent(const BaseContainer &msg)
{
	if(msg.GetLong(BFM_INPUT_DEVICE) == BFM_INPUT_MOUSE){
		if(msg.GetLong(BFM_INPUT_CHANNEL) == BFM_INPUT_MOUSELEFT){
			m_mouseX = msg.GetLong(BFM_INPUT_X);
			m_mouseY = msg.GetLong(BFM_INPUT_Y);
			m_mouseDown = TRUE;
			Global2Local(&m_mouseX, &m_mouseY);
			MouseDragStart(BFM_INPUT_MOUSELEFT,m_mouseX, m_mouseY,MOUSEDRAGFLAGS_0);
			MouseUpdate();
		}
	}
	Real x, y;
	BaseContainer channels;
	while (MouseDrag(&x, &y, &channels) == MOUSEDRAGRESULT_CONTINUE)
	{
		m_mouseX -= x;
		m_mouseY -= y;
		//Global2Local(&m_mouseX, &m_mouseY);
		MouseUpdate();
	}
	BaseContainer res;
	if(GetInputState(BFM_INPUT_MOUSE,BFM_INPUT_MOUSELEFT,res) && res.GetLong(BFM_INPUT_VALUE) == 0){
		if(m_mouseDown){
			MouseDragEnd();
			m_mouseDown = FALSE;
		}
	}

	return FALSE;
}


void ColorBox::SetColor(Vector color){
	m_color = color;
}

Vector ColorBox::GetColor(){
	return m_color;
}
