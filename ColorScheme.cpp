#include "colorscheme.h"
#include "colorwheel.h"

GeDynamicArray<ColorScheme*> ColorScheme::m_schemes;

void ColorScheme::Init(){
	m_schemes.Push(new TriadScheme());
	m_schemes.Push(new AdjacentScheme());
	m_schemes.Push(new MonoScheme());
	m_schemes.Push(new TetradScheme());
}
void ColorScheme::Free()
{
	for(LONG i=0;i<m_schemes.GetCount();i++){
		delete m_schemes[i];
	}
}
ColorScheme *ColorScheme::GetColorScheme(LONG id)
{
	return m_schemes[id];
}

// ------- Triad ----------

void TriadScheme::SetupOffsets(ColorWheel *wheel)
{
	GeDynamicArray<Real> offsets;
	offsets.Insert( 0.0,0);
	offsets.Insert( 0.4,1);
	offsets.Insert(-0.4,2);
	wheel->SetOffsets(offsets);
}

void TriadScheme::MarkerChanged(LONG marker, Real dist,GeDynamicArray<Real> &offsets)
{
	LONG otherMarker = marker == 1? 2 : 1;
	offsets[marker] = dist;
	offsets[otherMarker] = -dist;
}

// ------- Adjacent ----------

void AdjacentScheme::SetupOffsets(ColorWheel *wheel)
{
	GeDynamicArray<Real> offsets;
	offsets.Insert( 0.0,0);
	offsets.Insert( 0.1,1);
	offsets.Insert(-0.1,2);
	wheel->SetOffsets(offsets);
}

// ------- Mono ----------

void MonoScheme::SetupOffsets(ColorWheel *wheel)
{
	GeDynamicArray<Real> offsets;
	offsets.Insert( 0.0,0);
	wheel->SetOffsets(offsets);
}


// ------- Triad ----------

void TetradScheme::SetupOffsets(ColorWheel *wheel)
{
	GeDynamicArray<Real> offsets;
	offsets.Insert( 0.0,0);
	offsets.Insert( 0.1,1);
	offsets.Insert( 0.5,2);
	offsets.Insert(-0.4,3);
	wheel->SetOffsets(offsets);
}

void TetradScheme::MarkerChanged(LONG marker, Real dist,GeDynamicArray<Real> &offsets)
{
	if(marker != 2){
		offsets[marker] = dist;
		switch(marker){
			case 1:
				offsets[3] = -0.5 + offsets[1];
				break;
			case 3:
				offsets[1] =  0.5 + offsets[3];
				break;
		}
	}
}