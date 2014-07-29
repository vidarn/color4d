#include "colorwheel.h"
#include "colordialog.h"
#include "utils.h"

//const Float PI2			= 6.283185307179586476925286766559;
//const Float PI2_INV	    = 0.15915494309189533576888376337251;



ColorWheel::ColorWheel(ColorDialog *parent):
m_valueRadius(7.0), m_valuePosition(70)
{
	m_w = 200;
	m_h = 200;
	m_parent = parent;
	m_wheelClipMap = GeClipMap::Alloc();
	m_markerClipMap = GeClipMap::Alloc();
	m_canvas = GeClipMap::Alloc();
	m_color.SetSource(COLOR_SOURCE_WHEEL);
}

ColorWheel::~ColorWheel(void)
{
	GeClipMap::Free(m_wheelClipMap);
	GeClipMap::Free(m_markerClipMap);
	GeClipMap::Free(m_canvas);
}

Bool ColorWheel::Init(void)
{
	m_centerX = m_w*0.5;
	m_centerY = m_h*0.5;
	UpdateCircle();
	UpdateMarker();
	m_canvas->Init(m_w,m_h,32);
	return TRUE;
}

void ColorWheel::UpdateCircle()
{
	const Float innerRadius = 40;
	const Float outerRadius = 100;
	const Float aaBuffer = 2;
	const Float innerSeparator = 2;
	m_wheelClipMap->Init(m_w,m_h,32);
	m_wheelClipMap->BeginDraw();
	for(Int32 y=0;y<m_h;y++){
		for(Int32 x=0;x<m_w;x++){
			Vector col;
			Int32 r,g,b;
			GetColorRGB(COLOR_BG,r,g,b);
			col.x = r/255.0;
			col.y = g/255.0;
			col.z = b/255.0;
			Int32 dx = x-m_centerX;
			Int32 dy = y-m_centerY;
			Float dist = Sqrt(Float(dx*dx + dy*dy));
			if(dist < outerRadius-aaBuffer){
				col.x = col.y = col.z = 0.0;
			}
			if(dist > innerRadius+innerSeparator){
				if(dist < outerRadius){
					Float posX = dx/dist;
					Float posY = dy/dist;
					Float angle = ACos(posX);
					Float hue = angle*PI2_INV;
					if(posY > 0){
						hue = 1.0 - hue;
					}
					Float val = 1.0 - Smoothstep(outerRadius-aaBuffer,outerRadius,dist);
					val *= Smoothstep(innerRadius+innerSeparator,innerRadius+innerSeparator+aaBuffer,dist);
					col = Color(hue,1.0,0.5).SetSource(COLOR_SOURCE_WHEEL).Convert(COLOR_SOURCE_DISPLAY).AsVector()*val + col*(1.0-val);
				}
			}
			else{
				Float val = Smoothstep(innerRadius-aaBuffer,innerRadius,dist);
				col = m_color.Convert(COLOR_SOURCE_DISPLAY).AsVector()*(1.0-val) + col*val;
			}
			m_wheelClipMap->SetPixelRGBA(x,y,col[0]*255,col[1]*255,col[2]*255);
		}
	}
	m_wheelClipMap->EndDraw();
}

void ColorWheel::UpdateMarker()
{
	const Float aaBuffer = 2;
	const Float valueSeparator = 2;
	const Int32 w = m_valueRadius*2;
	const Int32 h = m_valueRadius*2;
	m_markerClipMap->Init(w,h,32);
	m_markerClipMap->BeginDraw();
	for(Int32 y=0;y<h;y++){
		for(Int32 x=0;x<w;x++){
			Float alpha = 0.0;
			Vector col = Vector(0.0, 0.0, 0.0);
			Float dx = x - m_valueRadius;
			Float dy = y - m_valueRadius;
			Float dist = Sqrt(dx*dx + dy*dy);
			if(dist < m_valueRadius){
				Float val = Smoothstep(m_valueRadius-aaBuffer,m_valueRadius,dist);
				col = Vector(0.0, 0.0, 0.0)*(1.0-val) + col*val;
				alpha = 1.0-val;
				if(dist < m_valueRadius-valueSeparator){
					Float val = 1.0 - Smoothstep(m_valueRadius-valueSeparator-aaBuffer,m_valueRadius-valueSeparator,dist);
					col = Vector(0.7,0.7,0.7)*val + col*(1.0-val);
				}
			}
			m_markerClipMap->SetPixelRGBA(x,y,col[0]*255,col[1]*255,col[2]*255,alpha*255);
		}
	}
	m_markerClipMap->EndDraw();
}

void ColorWheel::UpdateCanvas()
{
	m_canvas->BeginDraw();
	m_canvas->Blit(0,0,*m_wheelClipMap,0,0,m_wheelClipMap->GetBw(),m_wheelClipMap->GetBh(),GE_CM_BLIT_COPY);
	for(int i = 0; i < m_offsets.GetCount(); i++){
		Int32 alpha = 255;
		if(i>0){
			alpha = 120;
		}
		m_canvas->SetDrawMode(GE_CM_DRAWMODE_BLEND,alpha);
		Float val = m_color[0] + m_offsets[i];
		Int32 currX =  cos(val*PI2)*m_valuePosition+m_centerX;
		Int32 currY = -sin(val*PI2)*m_valuePosition+m_centerY;
		m_canvas->Blit(currX,currY,*m_markerClipMap,0,0,m_markerClipMap->GetBw(),m_markerClipMap->GetBh(),GE_CM_BLIT_COPY);
	}
	m_canvas->EndDraw();
}

Bool ColorWheel::GetMinSize(Int32 &w,Int32 &h)
{
	w = m_w;
	h = m_h;
	return TRUE;
}

void ColorWheel::Sized(Int32 w,Int32 h)
{
	m_w = w;
	m_h = h;
	m_centerX = m_w*0.5;
	m_centerY = m_h*0.5;
	m_canvas->Init(m_w,m_h,32);
	UpdateCircle();
	Redraw();
}

void ColorWheel::DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg)
{
	// skip the redraw in case if focus change
	Int32 reason = msg.GetInt32(BFM_DRAW_REASON);
	if (reason==BFM_GOTFOCUS || reason==BFM_LOSTFOCUS) 
		return;
	OffScreenOn();
	UpdateCanvas();
	BaseBitmap *bitmap = m_canvas->GetBitmap();
	DrawBitmap(bitmap,x1,y1,bitmap->GetBw(),bitmap->GetBh(),0,0,bitmap->GetBw(),bitmap->GetBh(),BMP_NORMAL);
}

void ColorWheel::UpdateColor(Color color){
	m_color = color;
	Redraw();
}

void ColorWheel::MouseUpdate(){
	Int32 dx = m_mouseX-m_centerX;
	Int32 dy = m_mouseY-m_centerY;
	Float dist = Sqrt(Float(dx*dx + dy*dy));
	Float x = dx;
	Float y = -dy;
	Float hue = ATan(y/x)*PI2_INV;
	if(x < 0){
		hue += 0.5;
	}
	hue = Wrap(hue,0.0,1.0);
	if(m_selectedMarker == 0){
		m_color[0] = hue;
	}
	else{
		m_offsets[m_selectedMarker] = hue - m_color[0];
	}
	m_parent->UpdateColor(m_color);
}

Bool ColorWheel::InputEvent(const BaseContainer &msg)
{
	if(msg.GetInt32(BFM_INPUT_DEVICE) == BFM_INPUT_MOUSE){
		if(msg.GetInt32(BFM_INPUT_CHANNEL) == BFM_INPUT_MOUSELEFT){
			m_mouseX = msg.GetInt32(BFM_INPUT_X);
			m_mouseY = msg.GetInt32(BFM_INPUT_Y);
			m_mouseDown = TRUE;
			Global2Local(&m_mouseX, &m_mouseY);
			MouseDragStart(BFM_INPUT_MOUSELEFT,m_mouseX, m_mouseY,MOUSEDRAGFLAGS_0);
			m_selectedMarker = 0;
			for(int i=0;i<m_offsets.GetCount();i++){
				Float val = m_color[0];
				val += m_offsets[i];
				Int32 currX =  cos(val*PI2)*m_valuePosition+m_centerX+m_valueRadius;
				Int32 currY = -sin(val*PI2)*m_valuePosition+m_centerY+m_valueRadius;
				Int32 dx = m_mouseX-currX;
				Int32 dy = m_mouseY-currY;
				Float dist = Sqrt(Float(dx*dx + dy*dy));
				if(dist <= m_valueRadius){
					m_selectedMarker = i;
				}
			}
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

void ColorWheel::SetOffsets(const GeDynamicArray<Float> &offsets)
{
	m_offsets = offsets;
}

void ColorWheel::GetOffsetColors(GeDynamicArray<Color> &colors)
{
	colors.FreeArray();
	Int32 num = m_offsets.GetCount();
	for(Int32 i=0;i<num;i++){
		Color col = m_color;
		col[0] = Wrap(col[0] + m_offsets[i],0.0,1.0);
		colors.Insert(col,i);
	}
}


void ColorWheel::SetColor(Color color){
	m_color = color;
}

Color ColorWheel::GetColor(){
	return m_color;
}
