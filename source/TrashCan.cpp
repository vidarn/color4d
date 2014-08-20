#include "trashcan.h"

BaseBitmap *TrashCan::m_trashIcon = NULL;

TrashCan::TrashCan()
{
	m_w = 16;
	m_h = 16;
    m_trashBitmap = NULL;
}

TrashCan::~TrashCan(void)
{
    if(m_trashBitmap != NULL){
        BaseBitmap::Free(m_trashBitmap);
    }
}

Bool TrashCan::Init(void)
{
    if(m_trashIcon == NULL){
        m_trashIcon = BaseBitmap::Alloc();
        m_trashIcon->Init(GeGetPluginPath() + Filename("res") + Filename("trash.tif"));
    }
    m_trashBitmap = BaseBitmap::Alloc();
    m_trashBitmap->Init(m_w,m_h,32);
	Int32 cr,cg,cb;
    GetColorRGB(COLOR_BG,cr,cg,cb);
	m_trashBitmap->Clear(cr,cg,cb);
	if(m_trashIcon != NULL){
		Int32 w = m_trashIcon->GetBw();
		Int32 h = m_trashIcon->GetBh();
		for(int y = 0; y < h; y++){
			for(int x=0; x < w; x++){
				UInt16 r, g, b, a;
				m_trashIcon->GetPixel(x,y,&r,&g,&b);
				m_trashIcon->GetAlphaPixel(m_trashIcon->GetInternalChannel(),x,y,&a);
				float aa = a/255.f;
				r = r*aa + cr*(1.f-aa);
				g = g*aa + cg*(1.f-aa);
				b = b*aa + cb*(1.f-aa);
				m_trashBitmap->SetPixel(x,y,r,g,b);
			}
		}
	}
	return TRUE;
}

Bool TrashCan::GetMinSize(Int32 &w,Int32 &h)
{
	w = 16;
	h = 16;
	return TRUE;
}

void TrashCan::Sized(Int32 w,Int32 h)
{
	m_w = w;
	m_h = h;
	Redraw();
}

void TrashCan::DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg)
{
	OffScreenOn();
    Vector tmp(0.5f, 0.5f, 0.5f);
	DrawSetPen(tmp);
	DrawRectangle(x1,y1,x2,y2);
    DrawBitmap(m_trashBitmap,x1,y1,m_trashBitmap->GetBw(),m_trashBitmap->GetBh(),0,0,m_trashBitmap->GetBw(),m_trashBitmap->GetBh(),BMP_NORMAL);
}

Bool TrashCan::InputEvent(const BaseContainer &msg)
{
	if(msg.GetInt32(BFM_INPUT_DEVICE) == BFM_INPUT_MOUSE){
		if(msg.GetInt32(BFM_INPUT_CHANNEL) == BFM_INPUT_MOUSELEFT){
            Vector col(-1.f,-1.f,-1.f);
            if(!HandleMouseDrag(msg,DRAGTYPE_RGB,&col,0)){
            }
		}
	}
	return FALSE;
}
