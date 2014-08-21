#pragma once
#include "c4d.h"

class TrashCan : public GeUserArea
{
public:
    
    TrashCan();
    ~TrashCan();
    
    virtual Bool Init(void);
    virtual Bool GetMinSize(Int32 &w,Int32 &h);
    virtual void Sized(Int32 w,Int32 h);
    virtual void DrawMsg(Int32 x1,Int32 y1,Int32 x2,Int32 y2, const BaseContainer &msg);
    virtual Bool InputEvent(const BaseContainer &msg);
    
    static BaseBitmap *m_trashIcon;
    BaseBitmap *m_trashBitmap;
    
    Int32 m_w;
    Int32 m_h;
};
