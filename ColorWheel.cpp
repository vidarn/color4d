#include "colorwheel.h"
#include "colorpickerdialog.h"

const Real PI2			= 6.283185307179586476925286766559;
const Real PI2_INV	    = 0.15915494309189533576888376337251;

ColorWheel::ColorWheel(ColorPickerDialog *parent)
{
	m_w = 200;
	m_h = 200;
	m_parent = parent;
	m_pixels = new UCHAR[m_w*3];
}

ColorWheel::~ColorWheel(void)
{
	BaseBitmap::Free(m_bitmap);
	delete m_pixels;
}

Bool ColorWheel::Init(void)
{
	m_centerX = m_w*0.5;
	m_centerY = m_h*0.5;
	m_color = (0.32,1.0,0.5);
	m_bitmap = BaseBitmap::Alloc();
	UpdateCircle();
	return TRUE;
}

void ColorWheel::UpdateCircle()
{
	const Real innerRadius = 40;
	const Real outerRadius = 100;
	const Real valuePosition = 70;
	const Real valueRadius = 7;
	const Real aaBuffer = 2;
	const Real innerSeparator = 2;
	const Real valueSeparator = 2;
	IMAGERESULT result = m_bitmap->Init(m_w,m_h,32,INITBITMAPFLAGS_0);
	for(LONG y=0;y<m_h;y++){
		for(LONG x=0;x<m_w;x++){
			Vector col;
			LONG r,g,b;
			GetColorRGB(COLOR_BG,r,g,b);
			col.x = r/255.0;
			col.y = g/255.0;
			col.z = b/255.0;
			LONG dx = x-m_centerX;
			LONG dy = y-m_centerY;
			Real dist = Sqrt(Real(dx*dx + dy*dy));
			if(dist < outerRadius-aaBuffer){
				col.x = col.y = col.z = 0.0;
			}
			if(dist > innerRadius+innerSeparator){
				if(dist < outerRadius){
					Real posX = dx/dist;
					Real posY = dy/dist;
					Real angle = ACos(posX);
					Real hue = angle*PI2_INV;
					if(posY > 0){
						hue = 1.0 - hue;
					}
					Real val = 1.0 - Smoothstep(outerRadius-aaBuffer,outerRadius,dist);
					val *= Smoothstep(innerRadius+innerSeparator,innerRadius+innerSeparator+aaBuffer,dist);
					col = HSLtoRGB(Vector(hue,1.0,0.5))*val + col*(1.0-val);
				}
			}
			else{
				Real val = Smoothstep(innerRadius-aaBuffer,innerRadius,dist);
				col = HSLtoRGB(m_color)*(1.0-val) + col*val;
			}
			LONG currX =  cos(m_color.x*PI2)*valuePosition+m_centerX;
			LONG currY = -sin(m_color.x*PI2)*valuePosition+m_centerY;
			dx = x - currX;
			dy = y - currY;
			dist = Sqrt(Real(dx*dx + dy*dy));
			if(dist < valueRadius){
				Real val = Smoothstep(valueRadius-aaBuffer,valueRadius,dist);
				col = Vector(0.0, 0.0, 0.0)*(1.0-val) + col*val;
				if(dist < valueRadius-valueSeparator){
					Real val = 1.0 - Smoothstep(valueRadius-valueSeparator-aaBuffer,valueRadius-valueSeparator,dist);
					col = Vector(0.7,0.7,0.7)*val + col*(1.0-val);
				}
			}
			m_pixels[x*3]   = 255*col.x;
			m_pixels[x*3+1] = 255*col.y;
			m_pixels[x*3+2] = 255*col.z;
		}
		m_bitmap->SetPixelCnt(0,y,m_w,m_pixels,3,COLORMODE_RGB,PIXELCNT_0);
	}
}

Bool ColorWheel::GetMinSize(LONG &w,LONG &h)
{
	w = m_w;
	h = m_h;
	return TRUE;
}

void ColorWheel::Sized(LONG w,LONG h)
{
	m_w = w;
	m_h = h;
	m_centerX = m_w*0.5;
	m_centerY = m_h*0.5;
	delete m_pixels;
	m_pixels = new UCHAR[m_w*3];
	UpdateCircle();
	Redraw();
}

void ColorWheel::DrawMsg(LONG x1,LONG y1,LONG x2,LONG y2, const BaseContainer &msg)
{
	// skip the redraw in case if focus change
	LONG reason = msg.GetLong(BFM_DRAW_REASON);
	if (reason==BFM_GOTFOCUS || reason==BFM_LOSTFOCUS) 
		return;
	OffScreenOn();
	DrawSetPen(COLOR_TEXT);
	DrawRectangle(30,30,30,30);
	DrawBitmap(m_bitmap,x1,y1,m_bitmap->GetBw(),m_bitmap->GetBh(),0,0,m_bitmap->GetBw(),m_bitmap->GetBh(),BMP_NORMAL);
}

void ColorWheel::UpdateColor(Vector color){
	m_color = color;
	UpdateCircle();
	Redraw();
}

void ColorWheel::MouseUpdate(){
	LONG dx = m_mouseX-m_centerX;
	LONG dy = m_mouseY-m_centerY;
	Real dist = Sqrt(Real(dx*dx + dy*dy));
	Real x = dx;
	Real y = -dy;
	Real hue = ATan(y/x)*PI2_INV;
	if(x < 0){
		hue += 0.5;
	}
	while(hue < 0.0){
		hue += 1.0;
	}
	while(hue > 1.0){
		hue -= 1.0;
	}
	m_color.x = hue;
	m_parent->UpdateColor(m_color);
}

Bool ColorWheel::InputEvent(const BaseContainer &msg)
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


void ColorWheel::SetColor(Vector color){
	m_color = color;
}

Vector ColorWheel::GetColor(){
	return m_color;
}
