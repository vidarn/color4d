#pragma once
#include "c4d.h"
#include "ge_dynamicarray.h"

class ColorWheel;

class ColorScheme
{
public:
    virtual void SetupOffsets(ColorWheel *wheel){}
    virtual void MarkerChanged(Int32 marker, Float dist, GeDynamicArray<Float> &offsets){}
    virtual String GetName(){return String("ColorScheme");}
    virtual Int32 GetNumMarkers(){return 0;}
    
    static void Init();
    static void Free();
    static ColorScheme *GetColorScheme(Int32 id);
    static Int32 GetNumSchemes(){return m_schemes.GetCount();}
    
private:
    static GeDynamicArray<ColorScheme*>  m_schemes;
};

class TriadScheme: public ColorScheme
{
    public:
    virtual void SetupOffsets(ColorWheel *wheel);
    virtual void MarkerChanged(Int32 marker, Float dist, GeDynamicArray<Float> &offsets);
    virtual String GetName(){return String("Triad");}
    virtual Int32 GetNumMarkers(){return 3;}
};

class AdjacentScheme: public TriadScheme
{
    public:
    virtual void SetupOffsets(ColorWheel *wheel);
    virtual String GetName(){return String("Adjacent");}
};

class MonoScheme: public ColorScheme
{
    public:
    virtual void SetupOffsets(ColorWheel *wheel);
    virtual String GetName(){return String("Mono");}
    virtual Int32 GetNumMarkers(){return 1;}
};

class TetradScheme: public ColorScheme
{
    public:
    virtual void SetupOffsets(ColorWheel *wheel);
    virtual void MarkerChanged(Int32 marker, Float dist, GeDynamicArray<Float> &offsets);
    virtual String GetName(){return String("Tetrad");}
    virtual Int32 GetNumMarkers(){return 4;}
};