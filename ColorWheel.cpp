#include "colorwheel.h"
#include "colordialog.h"
#include "colorwheelsubdialog.h"
#include "utils.h"

ColorWheel::ColorWheel(ColorDialog *parent, ColorWheelSubDialog *subParent):
m_valueRadius(7.0), m_valuePosition(90), m_scheme(nullptr), m_oldH(-1.f),m_type(COLOR_WHEEL_RECTANGLE)
{
	m_w = 200;
	m_h = 200;
    m_innerRadius = 80;
	m_outerRadius = 100;
    m_triangleW = Sqrt(m_innerRadius*m_innerRadius*2);
	m_parent = parent;
    m_subParent = subParent;
	m_wheelClipMap = GeClipMap::Alloc();
	m_markerClipMap = GeClipMap::Alloc();
    m_triangleClipMap = GeClipMap::Alloc();
	m_canvas = GeClipMap::Alloc();
	m_color.SetSource(COLOR_SOURCE_WHEEL);
}

ColorWheel::~ColorWheel(void)
{
	GeClipMap::Free(m_wheelClipMap);
	GeClipMap::Free(m_markerClipMap);
    GeClipMap::Free(m_triangleClipMap);
	GeClipMap::Free(m_canvas);
}

Bool ColorWheel::Init(void)
{
	m_centerX = m_w*0.5;
	m_centerY = m_h*0.5;
	UpdateCircle();
    UpdateTriangle();
	UpdateMarker();
	m_canvas->Init(m_w,m_h,32);
	return TRUE;
}

void ColorWheel::UpdateCircle()
{
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
            if(dist > m_innerRadius+innerSeparator || m_type == COLOR_WHEEL_BLENDER){
                if(dist < m_outerRadius){
                    Float hue, saturation, brightness = 1.0;
                    Float posX = dx/dist;
                    Float posY = dy/dist;
                    Float angle = ACos(posX);
                    Float val = 1.0 - Smoothstep(m_outerRadius-aaBuffer,m_outerRadius,dist);
                    if(Color::m_wheelType == WHEEL_TYPE_HSB){
                        brightness = 0.5;
                    }
                    if( Color::m_wheelType == WHEEL_TYPE_LCH){
                        brightness = m_color[2];
                    }
                    switch(m_type){
                    case COLOR_WHEEL_RECTANGLE:
                        saturation = 1.0;
                        hue = angle*PI2_INV;
                        if(posY > 0){
                            hue = 1.0 - hue;
                        }
                        val *= Smoothstep(m_innerRadius+innerSeparator,m_innerRadius+innerSeparator+aaBuffer,dist);
                        break;
                    case COLOR_WHEEL_BLENDER:
                        hue = angle*PI2_INV;
                        if(posY > 0){
                            hue = 1.0 - hue;
                        }
                        saturation = Pow(dist/m_outerRadius,2.0);
                        break;
                    }
                    col = Color(hue,saturation,brightness).SetSource(COLOR_SOURCE_WHEEL).Convert(COLOR_SOURCE_DISPLAY).AsVector()*val + col*(1.0-val);
                }
            }
            ClampColor(col);
            m_wheelClipMap->SetPixelRGBA(x,y,col[0]*255,col[1]*255,col[2]*255);
        }
    }
	m_wheelClipMap->EndDraw();
}

void ColorWheel::UpdateTriangle()
{
    if(m_type == COLOR_WHEEL_RECTANGLE){
        m_triangleClipMap->Init(m_triangleW,m_triangleW,32);
        m_triangleClipMap->BeginDraw();
        for(Int32 y=0;y<m_triangleW;y++){
            for(Int32 x=0;x<m_triangleW;x++){
                Vector col = Color(m_color[0],x/Float(m_triangleW),y/Float(m_triangleW)).SetSource(COLOR_SOURCE_WHEEL).Convert(COLOR_SOURCE_DISPLAY).AsVector();
                ClampColor(col);
                m_triangleClipMap->SetPixelRGBA(x,y,255*col.x,255*col.y,255*col.z);
            }
        }
        m_triangleClipMap->EndDraw();
    }
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
    if(m_type == COLOR_WHEEL_RECTANGLE){
        Float offsetX = (m_w - m_triangleW)*0.5;
        Float offsetY = (m_h - m_triangleW)*0.5;
        m_canvas->Blit(offsetX,offsetY,*m_triangleClipMap,0,0,m_triangleClipMap->GetBw(),m_triangleClipMap->GetBh(),GE_CM_BLIT_COPY);
        m_canvas->SetDrawMode(GE_CM_DRAWMODE_BLEND,255);
        Int32 currX =  m_color[1]*m_triangleW - m_triangleW*0.5 + m_centerX;
        Int32 currY =  m_color[2]*m_triangleW - m_triangleW*0.5 + m_centerY;
        m_canvas->Blit(currX-m_markerClipMap->GetBw()*0.5,currY-m_markerClipMap->GetBh()*0.5,*m_markerClipMap,0,0,m_markerClipMap->GetBw(),m_markerClipMap->GetBh(),GE_CM_BLIT_COPY);
    }
	for(int i = 0; i < m_offsets.GetCount(); i++){
		Int32 alpha = 255;
		if(i>0){
			alpha = 120;
		}
		m_canvas->SetDrawMode(GE_CM_DRAWMODE_BLEND,alpha);
		Float val = m_color[0] + m_offsets[i];
        Float valuePosition = m_valuePosition;
        if(m_type == COLOR_WHEEL_BLENDER){
            valuePosition = Pow(m_color[1],1.0/2.0)*m_outerRadius;
        }
		Int32 currX =  cos(val*PI2)*valuePosition+m_centerX;
		Int32 currY = -sin(val*PI2)*valuePosition+m_centerY;
		m_canvas->Blit(currX-m_markerClipMap->GetBw()*0.5,currY-m_markerClipMap->GetBh()*0.5,*m_markerClipMap,0,0,m_markerClipMap->GetBw(),m_markerClipMap->GetBh(),GE_CM_BLIT_COPY);
	}
    
    Int32 currX =  m_valueRadius;
    Int32 currY =  m_valueRadius;
    Int32 alpha = 120;
    if(m_type == COLOR_WHEEL_RECTANGLE){
        alpha = 255;
    }
    m_canvas->SetDrawMode(GE_CM_DRAWMODE_BLEND,alpha);
    m_canvas->Blit(currX-m_markerClipMap->GetBw()*0.5,currY-m_markerClipMap->GetBh()*0.5,*m_markerClipMap,0,0,m_markerClipMap->GetBw(),m_markerClipMap->GetBh(),GE_CM_BLIT_COPY);
    currX += m_valueRadius*2+1;
    alpha = 120;
    if(m_type == COLOR_WHEEL_BLENDER){
        alpha = 255;
    }
    m_canvas->SetDrawMode(GE_CM_DRAWMODE_BLEND,alpha);
    m_canvas->Blit(currX-m_markerClipMap->GetBw()*0.5,currY-m_markerClipMap->GetBh()*0.5,*m_markerClipMap,0,0,m_markerClipMap->GetBw(),m_markerClipMap->GetBh(),GE_CM_BLIT_COPY);
    
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
    ClampColor(color);
    Color tmp = color.Convert(COLOR_SOURCE_WHEEL);
    Float newH = tmp[0];
    Float newS = tmp[1];
    Float newV = tmp[2];
    m_color = color;
    if(m_oldH != newH){
        UpdateTriangle();
    }
    if(Color::m_wheelType == WHEEL_TYPE_LCH && newV != m_oldV){
        UpdateCircle();
    }
    m_oldH = newH;
    m_oldS = newS;
    m_oldV = newV;
	Redraw();
}

void ColorWheel::MouseUpdate(){
	Int32 dx = m_mouseX-m_centerX;
	Int32 dy = m_mouseY-m_centerY;
    if(m_mouseDragTriangle){
        m_mouseX = m_centerX + ClampValue(Float(dx), -m_triangleW*0.5, m_triangleW*0.5);
        m_mouseY = m_centerY + ClampValue(Float(dy), -m_triangleW*0.5, m_triangleW*0.5);
        Float saturation = ClampValue(0.5 + dx/Float(m_triangleW),0.0,1.0);
        m_color[1] = saturation;
        Float lightness = ClampValue(0.5 + dy/Float(m_triangleW),0.0,1.0);
        m_color[2] = lightness;
    }
    else{
        Float x = dx;
        Float y = -dy;
        Float hue = ATan(y/x)*PI2_INV;
        if(x < 0){
            hue += 0.5;
        }
        Float d = Sqrt(Sqr(dx)+Sqr(dy));
        Float dist = ClampValue(d/m_outerRadius, 0.0, 1.0);
        if(d > m_outerRadius){
            m_mouseX = dx/d*dist*m_outerRadius + m_centerX;
            m_mouseY = dy/d*dist*m_outerRadius + m_centerY;
        }
        hue = Wrap(hue,0.0,1.0);
        if(m_selectedMarker == 0){
            m_color[0] = hue;
        }
        else{
            if(m_scheme != nullptr){
                m_scheme->MarkerChanged(m_selectedMarker,hue-m_color[0], m_offsets);
            }
        }
        if(m_type == COLOR_WHEEL_BLENDER){
            Float saturation = dist;
            saturation = Pow(dist,2.0);
            m_color[1] = saturation;
        }
    }
	m_parent->UpdateColor(m_color);
}

Bool ColorWheel::CheckMarkerSelect(Int32 x, Int32 y)
{
    Int32 dx = m_mouseX-x;
    Int32 dy = m_mouseY-y;
    Float dist = Sqrt(Float(dx*dx + dy*dy));
    return dist <= m_valueRadius;
}

Bool ColorWheel::InputEvent(const BaseContainer &msg)
{
    Bool taken = false;
	if(msg.GetInt32(BFM_INPUT_DEVICE) == BFM_INPUT_MOUSE){
		if(msg.GetInt32(BFM_INPUT_CHANNEL) == BFM_INPUT_MOUSELEFT){
			m_mouseX = msg.GetInt32(BFM_INPUT_X);
			m_mouseY = msg.GetInt32(BFM_INPUT_Y);
			m_mouseDown = TRUE;
			Global2Local(&m_mouseX, &m_mouseY);
            {
                // Handle switching of palette type
                Int32 currX =  m_valueRadius;
                Int32 currY =  m_valueRadius;
                if(CheckMarkerSelect(currX, currY)){
                    m_type = COLOR_WHEEL_RECTANGLE;
                    taken = true;
                }
                currX += m_valueRadius*2+1;
                if(CheckMarkerSelect(currX, currY)){
                    m_type = COLOR_WHEEL_BLENDER;
                    taken = true;
                }
                if(taken){
                    UpdateCircle();
                    UpdateTriangle();
                    m_subParent->SetWheelType(m_type);
                    Redraw();
                }
            }
            if(!taken){
                MouseDragStart(BFM_INPUT_MOUSELEFT,m_mouseX, m_mouseY,MOUSEDRAGFLAGS_0);
                Int32 dx = m_mouseX-m_centerX;
                Int32 dy = m_mouseY-m_centerY;
                m_mouseDragTriangle = Abs(dx)*2 < m_triangleW && Abs(dy)*2 < m_triangleW && m_type == COLOR_WHEEL_RECTANGLE;
                m_selectedMarker = 0;
                float radius = m_valuePosition;
                if(m_type == COLOR_WHEEL_BLENDER){
                    radius = Pow(m_color[1],1.0/2.0)*m_outerRadius;
                }
                if(!m_mouseDragTriangle){
                    for(int i=0;i<m_offsets.GetCount();i++){
                        Float val = m_color[0];
                        val += m_offsets[i];
                        Int32 currX =  cos(val*PI2)*radius+m_centerX;
                        Int32 currY = -sin(val*PI2)*radius+m_centerY;
                        if(CheckMarkerSelect(currX, currY)){
                            m_selectedMarker = i;
                        }
                    }
                }
                MouseUpdate();
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
            }
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
    ClampColor(m_color);
}

void ColorWheel::SetScheme(ColorScheme *scheme){
	m_scheme = scheme;
	m_scheme->SetupOffsets(this);
}

Color ColorWheel::GetColor(){
	return m_color;
}
