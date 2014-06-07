#include "colorslider.h"
#include "colorpickerdialog.h"

ColorSlider::ColorSlider(ColorPickerDialog *parent, LONG index)
{
	ColorSlider();
	m_parent = parent;
	m_index = index;
}

ColorSlider::ColorSlider()
{
	m_w = 100;
	m_h = 15;
	m_parent = NULL;
	m_value = 0.0;
	m_index = 0;
	m_bitmap = BaseBitmap::Alloc();
}

ColorSlider::~ColorSlider(void)
{
	BaseBitmap::Free(m_bitmap);
}

Bool ColorSlider::Init(void)
{
	m_color = (0.32,1.0,0.5);
	UpdateCircle();
	return TRUE;
}

void ColorSlider::UpdateCircle()
{
	if(m_parent != NULL){
		const Real valueRadius = 7;
		const Real aaBuffer = 2;
		const Real valueSeparator = 2;
		IMAGERESULT result = m_bitmap->Init(m_w,m_h,32,INITBITMAPFLAGS_0);
		for(LONG y=0;y<m_h;y++){
			for(LONG x=0;x<m_w;x++){
				Vector col = m_color;
				UpdateColorWithValue(x/Real(m_w),col);
				col = m_parent->RGBSlidersTosRGB(col);
				LONG currX =  m_value*m_w;
				LONG currY = m_h/2;
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
}

Bool ColorSlider::GetMinSize(LONG &w,LONG &h)
{
	w = m_w;
	h = m_h;
	return TRUE;
}

void ColorSlider::Sized(LONG w,LONG h)
{
	m_w = w;
	m_h = h;
	m_centerX = m_w*0.5;
	m_centerY = m_h*0.5;
	UpdateCircle();
	Redraw();
}

void ColorSlider::DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg)
{
	// skip the redraw in case if focus change
	LONG reason = msg.GetLong(BFM_DRAW_REASON);
	if (reason==BFM_GOTFOCUS || reason==BFM_LOSTFOCUS) 
		return;
	OffScreenOn();
	DrawBitmap(m_bitmap,x1,y1,m_bitmap->GetBw(),m_bitmap->GetBh(),0,0,m_bitmap->GetBw(),m_bitmap->GetBh(),BMP_NORMAL);
}

void ColorSlider::UpdateColor(Vector color){
	SetColor(color);
	UpdateCircle();
	Redraw();
}

void ColorSlider::UpdateColorWithValue(Real value, Vector &color){
	switch(m_index){
		case 0:
			color.x = value;
			break;
		case 1:
			color.y = value;
			break;
		case 2:
			color.z = value;
			break;
	}
}

void ColorSlider::MouseUpdate(){
	m_value = Clamp(0.0,1.0,m_mouseX/Real(m_w));
	UpdateColorWithValue(m_value,m_color);
	if(m_parent != NULL){
		m_parent->UpdateColor(m_parent->RGBSlidersToLab(m_color));
	}
}

Bool ColorSlider::InputEvent(const BaseContainer &msg)
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


void ColorSlider::SetColor(Vector color){
	m_color = color;
	switch(m_index){
		case 0:
			m_value = color.x;
			break;
		case 1:
			m_value = color.y;
			break;
		case 2:
			m_value = color.z;
			break;
	}
}

void ColorSlider::SetParent(ColorPickerDialog *parent)
{
	m_parent = parent;
}

void ColorSlider::SetIndex(LONG index)
{
	m_index = index;
}

Vector ColorSlider::GetColor(){
	return m_color;
}
