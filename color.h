#pragma once
#include "c4d.h"
#include "main.h"
#include "lcms2.h"
#include "ge_dynamicarray.h"

enum COLOR_SOURCE
{
	COLOR_SOURCE_WHEEL,
	COLOR_SOURCE_RGB,
	COLOR_SOURCE_CMYK,
	COLOR_SOURCE_DISPLAY,
};

class vnColorProfile
{
	public:
		vnColorProfile():m_name(""),m_profile(0){};
		vnColorProfile(const String &name,cmsHPROFILE profile):m_name(name),m_profile(profile){};
		String m_name;
		cmsHPROFILE m_profile;
};

class Color
{
	public:
		Color();
		Color(Vector rgb);
		Color(Real r, Real g, Real b);
		Color(Real c, Real m, Real y, Real k);
		~Color();
		Real &c(){return m_val[0];}
		Real &m(){return m_val[1];}
		Real &y(){return m_val[2];}
		Real &k(){return m_val[3];}
		Real &r(){return m_val[0];}
		Real &g(){return m_val[1];}
		Real &b(){return m_val[2];}
		Real &operator[](int i){return m_val[i];}
		Color &SetSource(COLOR_SOURCE source){m_source = source; return *this;}
		COLOR_SOURCE GetSource(){return m_source;}
		Color Convert(COLOR_SOURCE source);
		Vector AsVector();
		static void SetWheelProfile(int profile, Bool updateTransform=FALSE);
		static void SetRGBProfile(int profile, Bool updateTransform=FALSE);
		static void SetCMYKProfile(int profile, Bool updateTransform=FALSE);
		static void SetDisplayProfile(int profile, Bool updateTransform=FALSE);
		static void SetWheelProfile(cmsHPROFILE profile, Bool updateTransform=FALSE);
		static void SetRGBProfile(cmsHPROFILE profile, Bool updateTransform=FALSE);
		static void SetCMYKProfile(cmsHPROFILE profile, Bool updateTransform=FALSE);
		static void SetDisplayProfile(cmsHPROFILE profile, Bool updateTransform=FALSE);
		static void UpdateTransforms();
		static void LoadICCProfiles();
		static const GeDynamicArray<vnColorProfile> &getRGBProfiles() {return m_RGBProfiles; }
		static const GeDynamicArray<vnColorProfile> &getCMYKProfiles(){return m_CMYKProfiles;}
		static const GeDynamicArray<vnColorProfile> &getSpotProfiles(){return m_spotProfiles;}
	private:
		Real m_val[4];
		COLOR_SOURCE m_source;
		static cmsHTRANSFORM m_wheelToRGB;
		static cmsHTRANSFORM m_wheelToCMYK;
		static cmsHTRANSFORM m_wheelToDisplay;
		static cmsHTRANSFORM m_RGBToWheel;
		static cmsHTRANSFORM m_RGBToCMYK;
		static cmsHTRANSFORM m_RGBToDisplay;
		static cmsHTRANSFORM m_CMYKToWheel;
		static cmsHTRANSFORM m_CMYKToRGB;
		static cmsHTRANSFORM m_CMYKToDisplay;
		static cmsHTRANSFORM m_displayToWheel;
		static cmsHTRANSFORM m_displayToRGB;
		static cmsHTRANSFORM m_displayToCMYK;
		static cmsHPROFILE m_wheelProfile, m_RGBProfile, m_CMYKProfile, m_displayProfile;
		static String *m_iccSearchPaths;
		static GeDynamicArray<vnColorProfile> m_RGBProfiles;
		static GeDynamicArray<vnColorProfile> m_CMYKProfiles;
		static GeDynamicArray<vnColorProfile> m_spotProfiles;
};
