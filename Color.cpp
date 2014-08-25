#include "color.h"

cmsHTRANSFORM Color::m_wheelToRGB     = 0;
cmsHTRANSFORM Color::m_wheelToCMYK    = 0;
cmsHTRANSFORM Color::m_wheelToDisplay = 0;
cmsHTRANSFORM Color::m_wheelToLAB     = 0;
cmsHTRANSFORM Color::m_RGBToWheel     = 0;
cmsHTRANSFORM Color::m_RGBToCMYK      = 0;
cmsHTRANSFORM Color::m_RGBToDisplay   = 0;
cmsHTRANSFORM Color::m_RGBToLAB       = 0;
cmsHTRANSFORM Color::m_CMYKToWheel    = 0;
cmsHTRANSFORM Color::m_CMYKToRGB      = 0;
cmsHTRANSFORM Color::m_CMYKToDisplay  = 0;
cmsHTRANSFORM Color::m_CMYKToLAB      = 0;
cmsHTRANSFORM Color::m_displayToWheel = 0;
cmsHTRANSFORM Color::m_displayToRGB   = 0;
cmsHTRANSFORM Color::m_displayToCMYK  = 0;
cmsHTRANSFORM Color::m_displayToLAB   = 0;
cmsHTRANSFORM Color::m_LABToWheel     = 0;
cmsHTRANSFORM Color::m_LABToRGB       = 0;
cmsHTRANSFORM Color::m_LABToCMYK      = 0;
cmsHTRANSFORM Color::m_LABToDisplay   = 0;
cmsHPROFILE   Color::m_wheelProfile   = 0;
cmsHPROFILE   Color::m_RGBProfile     = 0;
cmsHPROFILE   Color::m_CMYKProfile    = 0;
cmsHPROFILE   Color::m_displayProfile = 0;
cmsHPROFILE   Color::m_LABProfile     = 0;
String *Color::m_iccSearchPaths       = 0;
GeDynamicArray<vnColorProfile> Color::m_RGBProfiles  = GeDynamicArray<vnColorProfile>();
GeDynamicArray<vnColorProfile> Color::m_CMYKProfiles = GeDynamicArray<vnColorProfile>();
GeDynamicArray<vnColorProfile> Color::m_spotProfiles = GeDynamicArray<vnColorProfile>();

Color::Color()
{
	for(int i=0;i<4;i++){
		m_val[i] = 0.0;
	}
	m_iccSearchPaths = NULL;
}

Color::Color(Vector rgb)
{
	for(int i=0;i<3;i++){
		m_val[i] = rgb[i];
	}
	m_val[3] = 0.0;
	m_iccSearchPaths = NULL;
	m_source = COLOR_SOURCE_RGB;
}

Color::Color(Float r, Float g, Float b)
{
	m_val[0] = r;
	m_val[1] = g;
	m_val[2] = b;
	m_val[3] = 0.0;
	m_iccSearchPaths = NULL;
	m_source = COLOR_SOURCE_RGB;
}

Color::Color(Float c, Float m, Float y, Float k)
{
	m_val[0] = c;
	m_val[1] = m;
	m_val[2] = y;
	m_val[3] = k;
	m_iccSearchPaths = NULL;
	m_source = COLOR_SOURCE_CMYK;
}

Color::~Color()
{
	if(m_iccSearchPaths!= NULL){
		delete m_iccSearchPaths;
	}
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
		Vector tmp = HSVToRGB(Vector(in[0],in[1],in[2]));
		in[0] = tmp.x; in[1] = tmp.y; in[2] = tmp.z;
		if(target == COLOR_SOURCE_RGB)     cmsDoTransform(m_wheelToRGB,     in, out, 1);
		if(target == COLOR_SOURCE_CMYK)    cmsDoTransform(m_wheelToCMYK,    in, out, 1);
		if(target == COLOR_SOURCE_DISPLAY) cmsDoTransform(m_wheelToDisplay, in, out, 1);
        if(target == COLOR_SOURCE_LAB)     cmsDoTransform(m_wheelToLAB,     in, out, 1);
	}
	if(m_source == COLOR_SOURCE_RGB){
		if(target == COLOR_SOURCE_WHEEL)   cmsDoTransform(m_RGBToWheel,     in, out, 1);
		if(target == COLOR_SOURCE_CMYK)    cmsDoTransform(m_RGBToCMYK,      in, out, 1);
		if(target == COLOR_SOURCE_DISPLAY) cmsDoTransform(m_RGBToDisplay,   in, out, 1);
        if(target == COLOR_SOURCE_LAB)     cmsDoTransform(m_RGBToLAB,       in, out, 1);
	}
	if(m_source == COLOR_SOURCE_CMYK){
		if(target == COLOR_SOURCE_WHEEL)   cmsDoTransform(m_CMYKToWheel,    in, out, 1);
		if(target == COLOR_SOURCE_RGB)     cmsDoTransform(m_CMYKToRGB,      in, out, 1);
		if(target == COLOR_SOURCE_DISPLAY) cmsDoTransform(m_CMYKToDisplay,  in, out, 1);
        if(target == COLOR_SOURCE_LAB)     cmsDoTransform(m_CMYKToLAB,      in, out, 1);
	}
	if(m_source == COLOR_SOURCE_DISPLAY){
		if(target == COLOR_SOURCE_WHEEL)   cmsDoTransform(m_displayToWheel, in, out, 1);
		if(target == COLOR_SOURCE_RGB)     cmsDoTransform(m_displayToRGB,   in, out, 1);
		if(target == COLOR_SOURCE_CMYK)    cmsDoTransform(m_displayToCMYK,  in, out, 1);
        if(target == COLOR_SOURCE_LAB)     cmsDoTransform(m_displayToLAB,   in, out, 1);
	}
    if(m_source == COLOR_SOURCE_LAB){
        if(target == COLOR_SOURCE_RGB)     cmsDoTransform(m_LABToRGB,       in, out, 1);
		if(target == COLOR_SOURCE_WHEEL)   cmsDoTransform(m_LABToWheel,     in, out, 1);
		if(target == COLOR_SOURCE_DISPLAY) cmsDoTransform(m_LABToDisplay,   in, out, 1);
		if(target == COLOR_SOURCE_CMYK)    cmsDoTransform(m_LABToCMYK,      in, out, 1);
	}
	if(target == COLOR_SOURCE_WHEEL){
		Vector tmp = RGBToHSV(Vector(out[0],out[1],out[2]));
		out[0] = tmp.x;out[1] = tmp.y;out[2] = tmp.z;
	}
	return Color(out[0], out[1], out[2], out[3]).SetSource(target);
}

Vector Color::AsVector() const
{
	return Vector(m_val[0], m_val[1], m_val[2]);
}

static int findHexChar(const String::PChar &c, bool &valid)
{
	const String hex("0123456789ABCDEF");
	for(int i=0;i<hex.GetLength();i++){
		if(hex[i] == c){
			return i;
		}
	}
	valid = false;
	return -1;
}

static void cleanString(String &str)
{
	String tmp = str.ToUpper();
	str = "";
	const String hex("0123456789ABCDEF");
	for(int i=0;i<tmp.GetLength();i++){
		for(int ii=0;ii<hex.GetLength();ii++){
			if(hex[ii] == tmp[i]){
				str += " ";
				str[str.GetLength()-1] = hex[ii];
				break;
			}
		}
	}
}

bool Color::FromString(const String &str)
{
	String s = str;
	cleanString(s);
	if(s.GetLength() >= 6){
		Float val[3];
		bool valid = true;
		for(Int32 i=0;i<3;i++){
			val[i] = (findHexChar(s[i*2],valid)*16.0 + findHexChar(s[i*2+1],valid))/255.0;
		}
		if(valid){
			for(Int32 i=0;i<3;i++){
				m_val[i] = val[i];
			}
		}
		return valid;
	}
	return false;
}
void Color::ToString(String &str)
{
	str = "#000000";
	const String hex("0123456789ABCDEF");
	for(Int32 i=0;i<3;i++){
		unsigned char val = m_val[i]*255.0;
		str[i*2+1]   = hex[val/16];
		str[i*2+2] = hex[val%16];
	}
}

void Color::SetWheelProfile(int profile, Bool updateTransform)
{
	SetWheelProfile(m_RGBProfiles[profile].m_profile,updateTransform);
}

void Color::SetRGBProfile(int profile, Bool updateTransform)
{
	SetRGBProfile(m_RGBProfiles[profile].m_profile,updateTransform);
}

void Color::SetCMYKProfile(int profile, Bool updateTransform)
{
	SetCMYKProfile(m_CMYKProfiles[profile].m_profile,updateTransform);
}

void Color::SetDisplayProfile(int profile, Bool updateTransform)
{
	SetDisplayProfile(m_RGBProfiles[profile].m_profile,updateTransform);
}

static void createAndFreeTransform(cmsHTRANSFORM &transform, cmsHPROFILE prof, cmsUInt32Number InputFormat, cmsHPROFILE Output, cmsUInt32Number OutputFormat)
{
	if(transform != 0){
		cmsDeleteTransform(transform);
	}
	transform = cmsCreateTransform(prof,  InputFormat,  Output,    OutputFormat, INTENT_PERCEPTUAL,0);
}

void Color::SetWheelProfile(cmsHPROFILE profile, Bool updateTransform)
{
	m_wheelProfile = profile;
	if(updateTransform){
		createAndFreeTransform(m_wheelToRGB,     m_wheelProfile,  TYPE_RGB_DBL,  m_RGBProfile,    TYPE_RGB_DBL);
		createAndFreeTransform(m_wheelToCMYK,    m_wheelProfile,  TYPE_RGB_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL);
		createAndFreeTransform(m_wheelToDisplay, m_wheelProfile,  TYPE_RGB_DBL,  m_displayProfile,TYPE_RGB_DBL);
		createAndFreeTransform(m_wheelToLAB,     m_wheelProfile,  TYPE_RGB_DBL,  m_LABProfile,    TYPE_Lab_DBL);
		createAndFreeTransform(m_RGBToWheel,     m_RGBProfile,    TYPE_RGB_DBL,  m_wheelProfile,  TYPE_RGB_DBL);
		createAndFreeTransform(m_CMYKToWheel,    m_CMYKProfile,   TYPE_CMYK_DBL, m_wheelProfile,  TYPE_RGB_DBL);
		createAndFreeTransform(m_displayToWheel, m_displayProfile,TYPE_RGB_DBL,  m_wheelProfile,  TYPE_RGB_DBL);
		createAndFreeTransform(m_LABToWheel,     m_LABProfile,    TYPE_Lab_DBL,  m_wheelProfile,  TYPE_RGB_DBL);
	}
}

void Color::SetRGBProfile(cmsHPROFILE profile, Bool updateTransform)
{
	m_RGBProfile = profile;
	if(updateTransform){
		createAndFreeTransform(m_RGBToWheel,   m_RGBProfile,    TYPE_RGB_DBL,  m_wheelProfile,  TYPE_RGB_DBL);
		createAndFreeTransform(m_RGBToCMYK,    m_RGBProfile,    TYPE_RGB_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL);
		createAndFreeTransform(m_RGBToDisplay, m_RGBProfile,    TYPE_RGB_DBL,  m_displayProfile,TYPE_RGB_DBL);
		createAndFreeTransform(m_RGBToLAB,     m_RGBProfile,    TYPE_RGB_DBL,  m_LABProfile,    TYPE_Lab_DBL);
		createAndFreeTransform(m_wheelToRGB,   m_wheelProfile,  TYPE_RGB_DBL,  m_RGBProfile,    TYPE_RGB_DBL);
		createAndFreeTransform(m_CMYKToRGB,    m_CMYKProfile,   TYPE_CMYK_DBL, m_RGBProfile,    TYPE_RGB_DBL);
		createAndFreeTransform(m_displayToRGB, m_displayProfile,TYPE_RGB_DBL,  m_RGBProfile,    TYPE_RGB_DBL);
		createAndFreeTransform(m_LABToRGB,     m_LABProfile,    TYPE_Lab_DBL,  m_RGBProfile,    TYPE_RGB_DBL);
	}
}

void Color::SetCMYKProfile(cmsHPROFILE profile, Bool updateTransform)
{
	m_CMYKProfile = profile;
	if(updateTransform){
		createAndFreeTransform(m_CMYKToWheel,   m_CMYKProfile,   TYPE_CMYK_DBL, m_wheelProfile,  TYPE_RGB_DBL);
		createAndFreeTransform(m_CMYKToRGB,     m_CMYKProfile,   TYPE_CMYK_DBL, m_RGBProfile,    TYPE_RGB_DBL);
		createAndFreeTransform(m_CMYKToDisplay, m_CMYKProfile,   TYPE_CMYK_DBL, m_displayProfile,TYPE_RGB_DBL);
		createAndFreeTransform(m_CMYKToLAB,     m_CMYKProfile,   TYPE_CMYK_DBL, m_LABProfile,    TYPE_Lab_DBL);
		createAndFreeTransform(m_wheelToCMYK,   m_wheelProfile,  TYPE_RGB_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL);
		createAndFreeTransform(m_RGBToCMYK,     m_RGBProfile,    TYPE_RGB_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL);
		createAndFreeTransform(m_displayToCMYK, m_displayProfile,TYPE_RGB_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL);
		createAndFreeTransform(m_LABToCMYK,     m_LABProfile,    TYPE_Lab_DBL,  m_CMYKProfile,   TYPE_CMYK_DBL);
	}
}

void Color::SetDisplayProfile(cmsHPROFILE profile, Bool updateTransform)
{
	m_displayProfile = profile;
	if(updateTransform){
		createAndFreeTransform(m_displayToWheel, m_displayProfile, TYPE_RGB_DBL,  m_wheelProfile,   TYPE_RGB_DBL);
		createAndFreeTransform(m_displayToRGB,   m_displayProfile, TYPE_RGB_DBL,  m_RGBProfile,     TYPE_RGB_DBL);
		createAndFreeTransform(m_displayToCMYK,  m_displayProfile, TYPE_RGB_DBL,  m_CMYKProfile,    TYPE_CMYK_DBL);
		createAndFreeTransform(m_displayToLAB,   m_displayProfile, TYPE_RGB_DBL,  m_LABProfile,     TYPE_Lab_DBL);
		createAndFreeTransform(m_wheelToDisplay, m_wheelProfile,   TYPE_RGB_DBL,  m_displayProfile, TYPE_RGB_DBL);
		createAndFreeTransform(m_RGBToDisplay,   m_RGBProfile,     TYPE_RGB_DBL,  m_displayProfile, TYPE_RGB_DBL);
		createAndFreeTransform(m_CMYKToDisplay,  m_CMYKProfile,    TYPE_CMYK_DBL, m_displayProfile, TYPE_RGB_DBL);
		createAndFreeTransform(m_LABToDisplay,   m_LABProfile,     TYPE_Lab_DBL,  m_displayProfile, TYPE_RGB_DBL);
	}
}

void Color::SetLABProfile(cmsHPROFILE profile, Bool updateTransform)
{
	m_LABProfile = profile;
	if(updateTransform){
		createAndFreeTransform(m_LABToWheel,     m_LABProfile,     TYPE_Lab_DBL,  m_wheelProfile,   TYPE_RGB_DBL);
		createAndFreeTransform(m_LABToRGB,       m_LABProfile,     TYPE_Lab_DBL,  m_RGBProfile,     TYPE_RGB_DBL);
		createAndFreeTransform(m_LABToCMYK,      m_LABProfile,     TYPE_Lab_DBL,  m_CMYKProfile,    TYPE_CMYK_DBL);
		createAndFreeTransform(m_LABToDisplay,   m_LABProfile,     TYPE_Lab_DBL,  m_displayProfile, TYPE_RGB_DBL);
		createAndFreeTransform(m_wheelToLAB,     m_wheelProfile,   TYPE_RGB_DBL,  m_LABProfile,     TYPE_Lab_DBL);
		createAndFreeTransform(m_RGBToLAB,       m_RGBProfile,     TYPE_RGB_DBL,  m_LABProfile,     TYPE_Lab_DBL);
		createAndFreeTransform(m_CMYKToLAB,      m_CMYKProfile,    TYPE_CMYK_DBL, m_LABProfile,     TYPE_Lab_DBL);
		createAndFreeTransform(m_displayToLAB,   m_displayProfile, TYPE_RGB_DBL,  m_LABProfile,     TYPE_Lab_DBL);
	}
}

void Color::UpdateTransforms()
{
	createAndFreeTransform(m_wheelToRGB,     m_wheelProfile,   TYPE_RGB_DBL,  m_RGBProfile,     TYPE_RGB_DBL);
	createAndFreeTransform(m_wheelToCMYK,    m_wheelProfile,   TYPE_RGB_DBL,  m_CMYKProfile,    TYPE_CMYK_DBL);
	createAndFreeTransform(m_wheelToDisplay, m_wheelProfile,   TYPE_RGB_DBL,  m_displayProfile, TYPE_RGB_DBL);
	createAndFreeTransform(m_wheelToLAB,     m_wheelProfile,   TYPE_RGB_DBL,  m_LABProfile,     TYPE_Lab_DBL);
	createAndFreeTransform(m_RGBToWheel,     m_RGBProfile,     TYPE_RGB_DBL,  m_wheelProfile,   TYPE_RGB_DBL);
	createAndFreeTransform(m_RGBToCMYK,      m_RGBProfile,     TYPE_RGB_DBL,  m_CMYKProfile,    TYPE_CMYK_DBL);
	createAndFreeTransform(m_RGBToDisplay,   m_RGBProfile,     TYPE_RGB_DBL,  m_displayProfile, TYPE_RGB_DBL);
	createAndFreeTransform(m_RGBToLAB,       m_RGBProfile,     TYPE_RGB_DBL,  m_LABProfile,    TYPE_Lab_DBL);
	createAndFreeTransform(m_CMYKToWheel,    m_CMYKProfile,    TYPE_CMYK_DBL, m_wheelProfile,   TYPE_RGB_DBL);
	createAndFreeTransform(m_CMYKToRGB,      m_CMYKProfile,    TYPE_CMYK_DBL, m_RGBProfile,     TYPE_RGB_DBL);
	createAndFreeTransform(m_CMYKToDisplay,  m_CMYKProfile,    TYPE_CMYK_DBL, m_displayProfile, TYPE_RGB_DBL);
	createAndFreeTransform(m_CMYKToLAB,      m_CMYKProfile,    TYPE_CMYK_DBL, m_LABProfile,     TYPE_Lab_DBL);
	createAndFreeTransform(m_displayToWheel, m_displayProfile, TYPE_RGB_DBL,  m_wheelProfile,   TYPE_RGB_DBL);
	createAndFreeTransform(m_displayToRGB,   m_displayProfile, TYPE_RGB_DBL,  m_RGBProfile,     TYPE_RGB_DBL);
	createAndFreeTransform(m_displayToCMYK,  m_displayProfile, TYPE_RGB_DBL,  m_CMYKProfile,    TYPE_CMYK_DBL);
	createAndFreeTransform(m_displayToLAB,   m_displayProfile, TYPE_RGB_DBL,  m_LABProfile,     TYPE_Lab_DBL);
	createAndFreeTransform(m_LABToWheel,     m_LABProfile,     TYPE_Lab_DBL,  m_wheelProfile,   TYPE_RGB_DBL);
	createAndFreeTransform(m_LABToRGB,       m_LABProfile,     TYPE_Lab_DBL,  m_RGBProfile,     TYPE_RGB_DBL);
	createAndFreeTransform(m_LABToCMYK,      m_LABProfile,     TYPE_Lab_DBL,  m_CMYKProfile,    TYPE_CMYK_DBL);
	createAndFreeTransform(m_LABToDisplay,   m_LABProfile,     TYPE_Lab_DBL,  m_displayProfile, TYPE_RGB_DBL);
}

Bool Color::IsRGBProfileOk()
{
    return m_RGBToDisplay != 0;
}

void Color::LoadICCProfiles()
{
#pragma message("TODO: allow more icc search paths")
    
	if(m_iccSearchPaths!= NULL){
		delete m_iccSearchPaths;
	}
	m_iccSearchPaths = new String[2];
	m_iccSearchPaths[0] = "C:\\Windows\\System32\\Spool\\Drivers\\Color\\";
    m_iccSearchPaths[1] = "/Library/ColorSync/Profiles/";
    //m_iccSearchPaths[1] = "/Users/vidarn/Library/ColorSync";

	m_LABProfile = cmsCreateLab4Profile(NULL);

	m_displayProfile = cmsCreate_sRGBProfile();
	m_RGBProfiles.Insert(vnColorProfile("sRGB",m_displayProfile),0);

	BrowseFiles* bf = BrowseFiles::Alloc();

	Filename dir(m_iccSearchPaths[1]);
	bf->Init(dir,FALSE);
	int RGBPos  = m_RGBProfiles.GetCount();
	int CMYKPos = m_CMYKProfiles.GetCount();
	int spotPos = m_spotProfiles.GetCount();

	if (bf)
	{
		while (bf->GetNext())
		{
			Filename fileName = bf->GetFilename();
			fileName.SetDirectory(dir);
			String str = fileName.GetString();
			Char *buffer = new Char[str.GetCStringLen()+1];
			str.GetCString(buffer,str.GetCStringLen()+1);
            GePrint(buffer);
			cmsHPROFILE profile = cmsOpenProfileFromFile(buffer, "r");
			if(profile != NULL){
				cmsColorSpaceSignature sig = cmsGetColorSpace(profile);
				Int32 length = cmsGetProfileInfoASCII(profile,cmsInfoDescription,"en","US",NULL,0);
				Char *buffer2 = new Char[length];
				cmsGetProfileInfoASCII(profile,cmsInfoDescription,"en","US",buffer2,length);
				String info(buffer2);
				int pt = _cmsLCMScolorSpace(sig);
				if(PT_RGB == pt){
					m_RGBProfiles.Insert(vnColorProfile(info,profile),RGBPos);
					RGBPos++;
				}
				if(PT_CMYK == pt){
					cmsHTRANSFORM xform = cmsCreateTransform(profile,TYPE_NAMED_COLOR_INDEX,m_displayProfile,TYPE_RGB_DBL,INTENT_PERCEPTUAL,0);
					if(xform != NULL){
						cmsNAMEDCOLORLIST* colorList = cmsGetNamedColorList(xform);
						if(colorList != NULL){
							m_spotProfiles.Insert(vnColorProfile(info,profile),spotPos);
							spotPos++;
						}
						else{
							m_CMYKProfiles.Insert(vnColorProfile(info,profile),CMYKPos);
							CMYKPos++;
						}
					}
					cmsDeleteTransform(xform);
				}
				delete buffer2;
			}
            else{
                GePrint("Could not load ^");
            }
			delete buffer;
		}
	}
	BrowseFiles::Free(bf);
}

static void deleteTransform(cmsHTRANSFORM transform){
	if(transform != 0){
		cmsDeleteTransform(transform);
	}
}

void Color::Unload()
{
	for(int i=0;i<m_RGBProfiles.GetCount();i++){
		cmsCloseProfile(m_RGBProfiles[i].m_profile);
	}
	for(int i=0;i<m_CMYKProfiles.GetCount();i++){
		cmsCloseProfile(m_CMYKProfiles[i].m_profile);
	}
	for(int i=0;i<m_spotProfiles.GetCount();i++){
		cmsCloseProfile(m_spotProfiles[i].m_profile);
	}
    cmsCloseProfile(m_LABProfile);
	deleteTransform(m_wheelToRGB);
	deleteTransform(m_wheelToCMYK);
	deleteTransform(m_wheelToDisplay);
	deleteTransform(m_wheelToLAB);
	deleteTransform(m_RGBToWheel);
	deleteTransform(m_RGBToCMYK);
	deleteTransform(m_RGBToDisplay);
	deleteTransform(m_RGBToLAB);
	deleteTransform(m_CMYKToWheel);
	deleteTransform(m_CMYKToRGB);
	deleteTransform(m_CMYKToDisplay);
	deleteTransform(m_CMYKToLAB);
	deleteTransform(m_displayToWheel);
	deleteTransform(m_displayToRGB);
	deleteTransform(m_displayToCMYK);
	deleteTransform(m_displayToLAB);
	deleteTransform(m_LABToWheel);
	deleteTransform(m_LABToRGB);
	deleteTransform(m_LABToCMYK);
	deleteTransform(m_LABToDisplay);
}


const Color &Color::operator=(const Color &other)
{
	for(int i=0;i<4;i++){
		m_val[i] = other.m_val[i];
	}
	m_source = other.m_source;
	return *this;
}
