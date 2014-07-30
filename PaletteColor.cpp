#include "palettecolor.h"

BaseBitmap *PaletteColor::m_refreshIcon = 0;
BaseBitmap *PaletteColor::m_leftArrowIcon = 0;
BaseBitmap *PaletteColor::m_rightArrowIcon = 0;

PaletteColor::PaletteColor()
{
	m_hoverState = HOVER_NONE;
	m_normalBitmap     = NULL;
	m_hoverBitmap      = NULL;
	m_leftHoverBitmap  = NULL;
	m_rightHoverBitmap = NULL;
	m_color = Color(0.f,1.f,0.f);
	m_palette = 0;
	m_colorID = 0;
	m_selectCallback = NULL;
	m_selectCallbackData = NULL;
}

static void maybeFree(BaseBitmap *bmp){
	if(bmp!= NULL) BaseBitmap::Free(bmp);
}

PaletteColor::~PaletteColor()
{
	maybeFree(m_normalBitmap);
	maybeFree(m_hoverBitmap);
	maybeFree(m_leftHoverBitmap);
	maybeFree(m_rightHoverBitmap);
}


Bool PaletteColor::Init(void)
{
	m_normalBitmap = BaseBitmap::Alloc();
	m_hoverBitmap = BaseBitmap::Alloc();
	m_leftHoverBitmap = BaseBitmap::Alloc();
	m_rightHoverBitmap = BaseBitmap::Alloc();
	UpdateBitmaps();
	return TRUE;
}

void PaletteColor::Sized(Int32 w,Int32 h)
{
	m_w = w;
	m_h = h;
	UpdateBitmaps();
	Redraw();
}

void PaletteColor::DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg)
{
	OffScreenOn();
	BaseBitmap *bitmap = m_normalBitmap;
	switch(m_hoverState){
		case HOVER_CENTER:
			bitmap = m_hoverBitmap;
			break;
		case HOVER_LEFT:
			bitmap = m_leftHoverBitmap;
			break;
		case HOVER_RIGHT:
			bitmap = m_rightHoverBitmap;
			break;
	}
	DrawBitmap(bitmap,x1,y1,bitmap->GetBw(),bitmap->GetBh(),0,0,bitmap->GetBw(),bitmap->GetBh(),BMP_NORMAL);
}


static void updateBitmap(BaseBitmap *canvas, const Vector &col, Int32 m_w, Int32 m_h, Float horiz, BaseBitmap *icon, Int32 alpha)
{
	canvas->Init(m_w,m_h,32);
	UInt16 cr = col.x*255;
	UInt16 cg = col.y*255;
	UInt16 cb = col.z*255;
	canvas->Clear(cr,cg,cb);
	if(icon != NULL){
		Int32 w = icon->GetBw();
		Int32 h = icon->GetBh();
		for(int y = 0, yy = m_h/2-h/2; y < h; y++, yy++){
			for(int x=0, xx = m_w*horiz-w*horiz; x < w; x++, xx++){
				UInt16 r, g, b, a;
				icon->GetPixel(x,y,&r,&g,&b);
				icon->GetAlphaPixel(icon->GetInternalChannel(),x,y,&a);
				float aa = a/255.f;
				r = r*aa + cr*(1.f-aa);
				g = g*aa + cg*(1.f-aa);
				b = b*aa + cb*(1.f-aa);
				canvas->SetPixel(xx,yy,r,g,b);
			}
		}
	}
}

void PaletteColor::UpdateBitmaps(){
	Vector col = m_color.Convert(COLOR_SOURCE_DISPLAY).AsVector();
	Float dim = 0.7;
	updateBitmap(m_normalBitmap,        col,m_w,m_h,0.f, NULL,              0);
	updateBitmap(m_hoverBitmap,     col*dim,m_w,m_h,0.5f,m_refreshIcon,   255);
	updateBitmap(m_leftHoverBitmap, col*dim,m_w,m_h,0.f, m_leftArrowIcon, 255);
	updateBitmap(m_rightHoverBitmap,col*dim,m_w,m_h,1.f, m_rightArrowIcon,255);
}

void PaletteColor::UpdateColor(Color color){
	SetColor(color);
	UpdateBitmaps();
	Redraw();
}

Int32 PaletteColor::Message(const BaseContainer& msg, BaseContainer& result)
{
	Int32 res = GeUserArea::Message(msg, result);
	if(msg.GetId() == BFM_DRAGRECEIVE){
		Int32 type = 0;
		void *object = NULL;
		GetDragObject(msg, &type, &object);
		if(type == DRAGTYPE_RGB){
			Vector *color = static_cast<Vector*>(object);
<<<<<<< HEAD
			if(msg.GetInt32(BFM_DRAG_FINISHED)){
=======
			if(msg.GetLong(BFM_DRAG_FINISHED)){
				switch(m_hoverState){
					case HOVER_LEFT:
						Palette::InsertPaletteColor(m_palette, m_colorID, Color(*color).SetSource(COLOR_SOURCE_DISPLAY));
						break;
					case HOVER_RIGHT:
						Palette::InsertPaletteColor(m_palette, m_colorID+1, Color(*color).SetSource(COLOR_SOURCE_DISPLAY));
						break;
					case HOVER_CENTER:
						Palette::SetPaletteColor(m_palette, m_colorID, Color(*color).SetSource(COLOR_SOURCE_DISPLAY));
						break;
				}
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
				m_hoverState = HOVER_NONE;
			}
			else{
				if (msg.GetInt32(BFM_DRAG_LOST)){
					m_hoverState = HOVER_NONE;
					Redraw();
				}
				else{
					BaseContainer state;
					if(GetInputState(BFM_INPUT_MOUSE, BFM_INPUT_MOUSELEFT, state)){
						Float sideWidth = 0.2;
						Int32 x = state.GetInt32(BFM_INPUT_X);
						Int32 y = state.GetInt32(BFM_INPUT_Y);
						Global2Local(&x,&y);
						if(x < m_w*sideWidth){
							m_hoverState = HOVER_LEFT;
						}
						else{
							if(x > m_w*(1.0-sideWidth)){
								m_hoverState = HOVER_RIGHT;
							}
							else{
								m_hoverState = HOVER_CENTER;
							}
						}
					}
					Redraw();
					return SetDragDestination(MOUSE_POINT_HAND);
				}
			}
		}
	}

	return res;
}

<<<<<<< HEAD
Bool PaletteColor::CoreMessage(Int32 id, const BaseContainer& msg)
=======
Bool PaletteColor::InputEvent(const BaseContainer &msg)
{
	if(msg.GetLong(BFM_INPUT_DEVICE) == BFM_INPUT_MOUSE){
		if(msg.GetLong(BFM_INPUT_CHANNEL) == BFM_INPUT_MOUSELEFT){
			if(m_selectCallback != NULL){
				m_selectCallback(m_color,m_selectCallbackData);
			}
		}
	}
	return SpotColor::InputEvent(msg);
}

Bool PaletteColor::CoreMessage(LONG id, const BaseContainer& msg)
>>>>>>> 653517188f352a024a1dec4993f6159c9681dd65
{
    switch ( id )
    {
      case  PALETTE_ID:                                      // internal message
			/*Int32 color =  (Int32) msg.GetVoid( BFM_CORE_PAR1 );
			Int32 palette = (Int32) msg.GetVoid( BFM_CORE_PAR2 );
			if(color == m_colorID && palette == m_palette){
				Palette::GetPaletteColor(m_palette,m_colorID,m_color);
				UpdateBitmaps();
				Redraw();
			}*/
        break;
    }
    return GeUserArea::CoreMessage( id, msg );
}

static void loadBitmap(BaseBitmap *&bmp, const char *filename){
	bmp = BaseBitmap::Alloc();
	bmp->Init(GeGetPluginPath() + Filename("res") + Filename(filename));
}

void PaletteColor::LoadIcons(){
	loadBitmap(m_refreshIcon,"refresh.tif");
	loadBitmap(m_leftArrowIcon,"leftArrow.tif");
	loadBitmap(m_rightArrowIcon,"rightArrow.tif");
	//GePrint("Loaded icons!" + Int32ToString((Int32)m_refreshIcon) + " " + Int32ToString((Int32)m_leftArrowIcon) + " " + Int32ToString((Int32)m_rightArrowIcon));
	
}

void PaletteColor::UnloadIcons(){
	BaseBitmap::Free(m_refreshIcon);
	BaseBitmap::Free(m_leftArrowIcon);
	BaseBitmap::Free(m_rightArrowIcon);
	GePrint("Unloaded icons!");
}