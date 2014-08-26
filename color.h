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
    COLOR_SOURCE_LAB,
};

enum WHEEL_TYPE
{
    WHEEL_TYPE_HSV,
    WHEEL_TYPE_HSB,
    WHEEL_TYPE_LCH,
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
		Color(Float r, Float g, Float b);
		Color(Float c, Float m, Float y, Float k);
		~Color();
		Float &c(){return m_val[0];}
		Float &m(){return m_val[1];}
		Float &y(){return m_val[2];}
		Float &k(){return m_val[3];}
		Float &r(){return m_val[0];}
		Float &g(){return m_val[1];}
		Float &b(){return m_val[2];}
		Float &operator[](int i){return m_val[i];}
		const Color &operator=(const Color &other);
		Color &SetSource(COLOR_SOURCE source){m_source = source; return *this;}
		COLOR_SOURCE GetSource(){return m_source;}
		Color Convert(COLOR_SOURCE source);
		Vector AsVector() const;
		bool FromString(const String &str);
		void ToString(String &str);
		static void SetWheelProfile(int profile, Bool updateTransform=FALSE);
		static void SetRGBProfile(int profile, Bool updateTransform=FALSE);
		static void SetCMYKProfile(int profile, Bool updateTransform=FALSE);
		static void SetDisplayProfile(int profile, Bool updateTransform=FALSE);
		static void SetWheelProfile(cmsHPROFILE profile, Bool updateTransform=FALSE);
		static void SetRGBProfile(cmsHPROFILE profile, Bool updateTransform=FALSE);
		static void SetCMYKProfile(cmsHPROFILE profile, Bool updateTransform=FALSE);
		static void SetDisplayProfile(cmsHPROFILE profile, Bool updateTransform=FALSE);
        static void SetLABProfile(cmsHPROFILE profile, Bool updateTransform=FALSE);
        static Bool IsRGBProfileOk();
		static void UpdateTransforms();
		static void LoadICCProfiles();
		static void Unload();
		static const GeDynamicArray<vnColorProfile> &getRGBProfiles() {return m_RGBProfiles; }
		static const GeDynamicArray<vnColorProfile> &getCMYKProfiles(){return m_CMYKProfiles;}
		static const GeDynamicArray<vnColorProfile> &getSpotProfiles(){return m_spotProfiles;}
		static cmsHPROFILE getDisplayProfile(){return m_displayProfile;}

		Float m_val[4];
		COLOR_SOURCE m_source;
		static cmsHTRANSFORM m_wheelToRGB;
		static cmsHTRANSFORM m_wheelToCMYK;
		static cmsHTRANSFORM m_wheelToDisplay;
        static cmsHTRANSFORM m_wheelToLAB;
		static cmsHTRANSFORM m_RGBToWheel;
		static cmsHTRANSFORM m_RGBToCMYK;
		static cmsHTRANSFORM m_RGBToDisplay;
        static cmsHTRANSFORM m_RGBToLAB;
		static cmsHTRANSFORM m_CMYKToWheel;
		static cmsHTRANSFORM m_CMYKToRGB;
		static cmsHTRANSFORM m_CMYKToDisplay;
        static cmsHTRANSFORM m_CMYKToLAB;
		static cmsHTRANSFORM m_displayToWheel;
		static cmsHTRANSFORM m_displayToRGB;
		static cmsHTRANSFORM m_displayToCMYK;
        static cmsHTRANSFORM m_displayToLAB;
        static cmsHTRANSFORM m_LABToWheel;
        static cmsHTRANSFORM m_LABToRGB;
        static cmsHTRANSFORM m_LABToCMYK;
        static cmsHTRANSFORM m_LABToDisplay;
		static cmsHPROFILE m_wheelProfile, m_RGBProfile, m_CMYKProfile, m_displayProfile, m_LABProfile;
		static String *m_iccSearchPaths;
		static GeDynamicArray<vnColorProfile> m_RGBProfiles;
		static GeDynamicArray<vnColorProfile> m_CMYKProfiles;
		static GeDynamicArray<vnColorProfile> m_spotProfiles;
        static Int32 m_wheelType;
        static cmsUInt32Number m_wheelDataType;
};
