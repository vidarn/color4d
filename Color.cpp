#include "color.h"

cmsHTRANSFORM Color::m_wheelToRGB     = 0;
cmsHTRANSFORM Color::m_wheelToCMYK    = 0;
cmsHTRANSFORM Color::m_wheelToDisplay = 0;
cmsHTRANSFORM Color::m_RGBToWheel     = 0;
cmsHTRANSFORM Color::m_RGBToCMYK      = 0;
cmsHTRANSFORM Color::m_RGBToDisplay   = 0;
cmsHTRANSFORM Color::m_CMYKToWheel    = 0;
cmsHTRANSFORM Color::m_CMYKToRGB      = 0;
cmsHTRANSFORM Color::m_CMYKToDisplay  = 0;
cmsHTRANSFORM Color::m_displayToWheel = 0;
cmsHTRANSFORM Color::m_displayToRGB   = 0;
cmsHTRANSFORM Color::m_displayToCMYK  = 0;
cmsHPROFILE   Color::m_wheelProfile   = 0;
cmsHPROFILE   Color::m_RGBProfile     = 0;
cmsHPROFILE   Color::m_CMYKProfile    = 0;
cmsHPROFILE   Color::m_displayProfile = 0;

Color::Color()
{
	for(int i=0;i<4;i++){
		m_val[i] = 0.0;
	}
}

Color::Color(Vector rgb)
{
	for(int i=0;i<3;i++){
		m_val[i] = rgb[i];
	}
	m_val[3] = 0.0;
}

Color::Color(Real r, Real g, Real b)
{
	m_val[0] = r;
	m_val[1] = g;
	m_val[2] = b;
	m_val[3] = 0.0;
}

Color::Color(Real c, Real m, Real y, Real k)
{
	m_val[0] = c;
	m_val[1] = m;
	m_val[2] = y;
	m_val[3] = k;
}

Color::~Color()
{
}

Color Color::Convert(COLOR_SOURCE target)
{
	double in[4], out[4];
	if(m_source == target){
		return *this;
	}
	for(int i=0;i<4;i++){
			in[i]  = m_val[i];
			out[i] = 0.0;
		}
	if(m_source == COLOR_SOURCE_WHEEL){
		Vector tmp = HSLtoRGB(Vector(in[0],in[1],in[2]));
		in[0] = tmp.x; in[1] = tmp.y; in[2] = tmp.z;
		if(target == COLOR_SOURCE_RGB)     cmsDoTransform(m_wheelToRGB,     in, out, 1);
		if(target == COLOR_SOURCE_CMYK)    cmsDoTransform(m_wheelToCMYK,    in, out, 1);
		if(target == COLOR_SOURCE_DISPLAY) cmsDoTransform(m_wheelToDisplay, in, out, 1);
	}
	if(m_source == COLOR_SOURCE_RGB){
		if(target == COLOR_SOURCE_WHEEL)   cmsDoTransform(m_RGBToWheel,     in, out, 1);
		if(target == COLOR_SOURCE_CMYK)    cmsDoTransform(m_RGBToCMYK,      in, out, 1);
		if(target == COLOR_SOURCE_DISPLAY) cmsDoTransform(m_RGBToDisplay,   in, out, 1);
	}
	if(m_source == COLOR_SOURCE_CMYK){
		if(target == COLOR_SOURCE_WHEEL)   cmsDoTransform(m_CMYKToWheel,    in, out, 1);
		if(target == COLOR_SOURCE_RGB)     cmsDoTransform(m_CMYKToRGB,      in, out, 1);
		if(target == COLOR_SOURCE_DISPLAY) cmsDoTransform(m_CMYKToDisplay,  in, out, 1);
	}
	if(m_source == COLOR_SOURCE_DISPLAY){
		if(target == COLOR_SOURCE_WHEEL)   cmsDoTransform(m_displayToWheel, in, out, 1);
		if(target == COLOR_SOURCE_RGB)     cmsDoTransform(m_displayToRGB,   in, out, 1);
		if(target == COLOR_SOURCE_CMYK)    cmsDoTransform(m_displayToCMYK,  in, out, 1);
	}
	if(target == COLOR_SOURCE_WHEEL){
		Vector tmp = RGBToHSL(Vector(out[0],out[1],out[2]));
		out[0] = tmp.x;out[1] = tmp.y;out[2] = tmp.z;
	}
	return Color(out[0], out[1], out[2], out[3]).SetSource(target);
}

Vector Color::AsVector(){
	return Vector(m_val[0], m_val[1], m_val[2]);
}

void Color::SetWheelProfile(cmsHPROFILE profile, Bool updateTransform)
{
	m_wheelProfile = profile;
	if(updateTransform){
		m_wheelToRGB =     cmsCreateTransform(m_wheelProfile,  TYPE_RGB_DBL,  m_RGBProfile,    TYPE_RGB_DBL, INTENT_PERCEPTUAL,0);
		m_wheelToCMYK =    cmsCreateTransform(m_wheelProfile,  TYPE_RGB_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL,INTENT_PERCEPTUAL,0);
		m_wheelToDisplay = cmsCreateTransform(m_wheelProfile,  TYPE_RGB_DBL,  m_displayProfile,TYPE_RGB_DBL, INTENT_PERCEPTUAL,0);
		m_RGBToWheel =	   cmsCreateTransform(m_RGBProfile,    TYPE_RGB_DBL,  m_wheelProfile,  TYPE_RGB_DBL, INTENT_PERCEPTUAL,0);
		m_CMYKToWheel =    cmsCreateTransform(m_CMYKProfile,   TYPE_CMYK_DBL, m_wheelProfile,  TYPE_RGB_DBL, INTENT_PERCEPTUAL,0);
		m_displayToWheel = cmsCreateTransform(m_displayProfile,TYPE_RGB_DBL,  m_wheelProfile,  TYPE_RGB_DBL, INTENT_PERCEPTUAL,0);
	}
}

void Color::SetRGBProfile(cmsHPROFILE profile, Bool updateTransform)
{
	m_RGBProfile = profile;
	if(updateTransform){
		m_RGBToWheel =   cmsCreateTransform(m_RGBProfile,    TYPE_RGB_DBL,  m_wheelProfile,  TYPE_RGB_DBL, INTENT_PERCEPTUAL,0);
		m_RGBToCMYK =    cmsCreateTransform(m_RGBProfile,    TYPE_RGB_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL,INTENT_PERCEPTUAL,0);
		m_RGBToDisplay = cmsCreateTransform(m_RGBProfile,    TYPE_RGB_DBL,  m_displayProfile,TYPE_RGB_DBL, INTENT_PERCEPTUAL,0);
		m_wheelToRGB =	 cmsCreateTransform(m_wheelProfile,  TYPE_RGB_DBL,  m_RGBProfile,    TYPE_RGB_DBL, INTENT_PERCEPTUAL,0);
		m_CMYKToRGB =    cmsCreateTransform(m_CMYKProfile,   TYPE_CMYK_DBL, m_RGBProfile,    TYPE_RGB_DBL, INTENT_PERCEPTUAL,0);
		m_displayToRGB = cmsCreateTransform(m_displayProfile,TYPE_RGB_DBL,  m_RGBProfile,    TYPE_RGB_DBL, INTENT_PERCEPTUAL,0);
	}
}

void Color::SetCMYKProfile(cmsHPROFILE profile, Bool updateTransform)
{
	m_CMYKProfile = profile;
	if(updateTransform){
		m_CMYKToWheel =   cmsCreateTransform(m_CMYKProfile,   TYPE_CMYK_DBL, m_wheelProfile,  TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
		m_CMYKToRGB =     cmsCreateTransform(m_CMYKProfile,   TYPE_CMYK_DBL, m_RGBProfile,    TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
		m_CMYKToDisplay = cmsCreateTransform(m_CMYKProfile,   TYPE_CMYK_DBL, m_displayProfile,TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
		m_wheelToCMYK =	  cmsCreateTransform(m_wheelProfile,  TYPE_RGB_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL, INTENT_PERCEPTUAL,0);
		m_RGBToCMYK =     cmsCreateTransform(m_RGBProfile,    TYPE_RGB_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL, INTENT_PERCEPTUAL,0);
		m_displayToCMYK = cmsCreateTransform(m_displayProfile,TYPE_RGB_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL, INTENT_PERCEPTUAL,0);
	}
}

void Color::SetDisplayProfile(cmsHPROFILE profile, Bool updateTransform)
{
	m_displayProfile = profile;
	if(updateTransform){
		m_displayToWheel =  cmsCreateTransform(m_displayProfile, TYPE_RGB_DBL,  m_wheelProfile,   TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
		m_displayToRGB =    cmsCreateTransform(m_displayProfile, TYPE_RGB_DBL,  m_RGBProfile,     TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
		m_displayToCMYK =   cmsCreateTransform(m_displayProfile, TYPE_RGB_DBL,  m_CMYKProfile,    TYPE_CMYK_DBL, INTENT_PERCEPTUAL,0);
		m_wheelToDisplay =	cmsCreateTransform(m_wheelProfile,   TYPE_RGB_DBL,  m_displayProfile, TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
		m_RGBToDisplay =    cmsCreateTransform(m_RGBProfile,     TYPE_RGB_DBL,  m_displayProfile, TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
		m_CMYKToDisplay =   cmsCreateTransform(m_CMYKProfile,    TYPE_CMYK_DBL, m_displayProfile, TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
	}
}

void Color::UpdateTransforms()
{
	m_wheelToRGB =     cmsCreateTransform(m_wheelProfile,   TYPE_RGB_DBL,  m_RGBProfile,     TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
	m_wheelToCMYK =    cmsCreateTransform(m_wheelProfile,   TYPE_RGB_DBL,  m_CMYKProfile,    TYPE_CMYK_DBL, INTENT_PERCEPTUAL,0);
	m_wheelToDisplay = cmsCreateTransform(m_wheelProfile,   TYPE_RGB_DBL,  m_displayProfile, TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
	m_RGBToWheel =     cmsCreateTransform(m_RGBProfile,     TYPE_RGB_DBL,  m_wheelProfile,   TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
	m_RGBToCMYK =      cmsCreateTransform(m_RGBProfile,     TYPE_RGB_DBL,  m_CMYKProfile,    TYPE_CMYK_DBL, INTENT_PERCEPTUAL,0);
	m_RGBToDisplay =   cmsCreateTransform(m_RGBProfile,     TYPE_RGB_DBL,  m_displayProfile, TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
	m_CMYKToWheel =    cmsCreateTransform(m_CMYKProfile,    TYPE_CMYK_DBL, m_wheelProfile,   TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
	m_CMYKToRGB =      cmsCreateTransform(m_CMYKProfile,    TYPE_CMYK_DBL, m_RGBProfile,     TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
	m_CMYKToDisplay =  cmsCreateTransform(m_CMYKProfile,    TYPE_CMYK_DBL, m_displayProfile, TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
	m_displayToWheel = cmsCreateTransform(m_displayProfile, TYPE_RGB_DBL,  m_wheelProfile,   TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
	m_displayToRGB =   cmsCreateTransform(m_displayProfile, TYPE_RGB_DBL,  m_RGBProfile,     TYPE_RGB_DBL,  INTENT_PERCEPTUAL,0);
	m_displayToCMYK =  cmsCreateTransform(m_displayProfile, TYPE_RGB_DBL,  m_CMYKProfile,    TYPE_CMYK_DBL, INTENT_PERCEPTUAL,0);
}