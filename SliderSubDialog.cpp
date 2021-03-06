#include "slidersubdialog.h"
#include "main.h"
#include "c4d_symbols.h"
#include "colorpickerdialog.h"
#include "utils.h"

#ifdef _WINDOWS
#include "Windows.h"
static HHOOK g_hook = 0;
static bool g_hookActive = false;

#else
#include "osxcolorfromscreen.h"
#endif

static SliderSubDialog *gColorPickDialog = NULL;

SliderSubDialog::SliderSubDialog()
{

}

SliderSubDialog::~SliderSubDialog()
{

}

Bool SliderSubDialog::CreateLayout(void)
{
    if (!GeDialog::CreateLayout()) return FALSE;

	GroupBegin(1,BFH_SCALEFIT,2,0,String(),0);
        GroupBegin(3,BFH_SCALEFIT|BFV_TOP,1,0,String("RGB"),BFV_BORDERGROUP_FOLD_OPEN);
            GroupBorder(BORDER_WITH_TITLE|BORDER_THIN_IN);
            GroupBegin(4,BFH_SCALEFIT,1,2,String(),0);
                iccRGBCombo  = AddComboBox(IDC_RGBICC,BFH_SCALEFIT,10,10);
            GroupEnd();
            GroupBegin(5,BFH_SCALEFIT,2,0,String(),0);
                for(Int32 i=0;i<3;i++){
                    RGBeditNumber[i] = AddEditNumberArrows(IDC_R+i,BFH_LEFT);
                    RGBsliderArea[i] = AddUserArea(IDC_RSLIDER+i,BFH_SCALEFIT);	
                    m_RGBSlider[i].SetParent(m_parent);
                    m_RGBSlider[i].SetIndex(i);
                    m_RGBSlider[i].SetColorSource(COLOR_SOURCE_RGB);
                    if (RGBsliderArea[i]){
                        AttachUserArea(m_RGBSlider[i],RGBsliderArea[i]);
                    }
                }
            GroupEnd();
        GroupEnd();
        GroupBegin(8,BFH_SCALEFIT|BFV_TOP,1,0,String("CMYK"),BFV_BORDERGROUP_FOLD_OPEN);
            GroupBorder(BORDER_WITH_TITLE|BORDER_THIN_IN);
            iccCMYKCombo = AddComboBox(IDC_CMYKICC,BFH_SCALEFIT,10,10);
            GroupBegin(9,BFH_SCALEFIT,2,0,String(),0);
                for(Int32 i=0;i<4;i++){
                    CMYKeditNumber[i] = AddEditNumberArrows(IDC_C+i,BFH_LEFT);
                    CMYKsliderArea[i] = AddUserArea(IDC_CSLIDER+i,BFH_SCALEFIT);	
                    m_CMYKSlider[i].SetParent(m_parent);
                    m_CMYKSlider[i].SetIndex(i);
                    m_CMYKSlider[i].SetColorSource(COLOR_SOURCE_CMYK);
                    m_CMYKSlider[i].SetValueMax(100.0);
                    if (CMYKsliderArea[i]){
                        AttachUserArea(m_CMYKSlider[i],CMYKsliderArea[i]);
                    }
                }
            GroupEnd();
        GroupEnd();
        GroupBegin(6,BFH_SCALEFIT|BFV_BOTTOM,1,0,String("HSV"),BFV_BORDERGROUP_FOLD_OPEN);
            GroupBorder(BORDER_WITH_TITLE|BORDER_THIN_IN);
            GroupBegin(4,BFH_SCALEFIT,1,2,String(),0);
                iccHSVCombo  = AddComboBox(IDC_HSVICC,BFH_SCALEFIT,10,10);
            GroupEnd();
            GroupBegin(7,BFH_SCALEFIT,2,0,String(),0);
                for(Int32 i=0;i<3;i++){
                    HSVeditNumber[i] = AddEditNumberArrows(IDC_H+i,BFH_LEFT);
                    HSVsliderArea[i] = AddUserArea(IDC_HSLIDER+i,BFH_SCALEFIT);
                    m_HSVSlider[i].SetParent(m_parent);
                    m_HSVSlider[i].SetIndex(i);
                    m_HSVSlider[i].SetColorSource(COLOR_SOURCE_WHEEL);
                    if (HSVsliderArea[i]){
                        AttachUserArea(m_HSVSlider[i],HSVsliderArea[i]);
                    }
                }
            GroupEnd();
        GroupEnd();
        GroupBegin(10,BFH_SCALEFIT|BFV_BOTTOM,1,0,String("LAB"),BFV_BORDERGROUP_FOLD_OPEN);
            GroupBorder(BORDER_WITH_TITLE|BORDER_THIN_IN);
            GroupBegin(11,BFH_SCALEFIT,2,0,String(),0);
                for(Int32 i=0;i<3;i++){
                    LABeditNumber[i] = AddEditNumberArrows(IDC_L+i,BFH_LEFT);
                    LABsliderArea[i] = AddUserArea(IDC_LSLIDER+i,BFH_SCALEFIT);
                    m_LABSlider[i].SetParent(m_parent);
                    m_LABSlider[i].SetIndex(i);
                    m_LABSlider[i].SetColorSource(COLOR_SOURCE_LAB);
                    if (LABsliderArea[i]){
                        AttachUserArea(m_LABSlider[i],LABsliderArea[i]);
                    }
                }
                m_LABSlider[0].SetValueMax(100.0);
                m_LABSlider[1].SetValueMax(128.0);
                m_LABSlider[1].m_valueMin = -128.0;
                m_LABSlider[2].SetValueMax(128.0);
                m_LABSlider[2].m_valueMin = -128.0;
            GroupEnd();
        GroupEnd();
		m_numberRangeCombo = AddComboBox(IDC_NUMBERRANGE,BFH_SCALEFIT,10,10);
		m_hexText = AddEditText(IDC_HEXTEXT,BFH_SCALEFIT);
		AddButton(IDC_SCREEN_PICK_BUTTTON,BFH_RIGHT,0,0,String("Pick color from screen"));
	GroupEnd();
    return TRUE;
}

Bool SliderSubDialog::InitValues(void)
{
	FindICCProfiles();
	BaseContainer numberRangeBC;
	numberRangeBC.SetString(0,"0 - 255");
	numberRangeBC.SetString(1,"0 - 1.0");
	numberRangeBC.SetString(2,"0 - 100.0");
	AddChildren(m_numberRangeCombo,numberRangeBC);
	SetInt32(m_numberRangeCombo,0);
	Enable(m_numberRangeCombo,TRUE);
	UpdateColor(Color(*m_pColor).SetSource(COLOR_SOURCE_DISPLAY));
    if (!GeDialog::InitValues()) return FALSE;
    return TRUE;
}

#ifdef _WINDOWS
static LRESULT CALLBACK MouseHookCallback(
  _In_  int nCode,
  _In_  WPARAM wParam,
  _In_  LPARAM lParam
){
	if(nCode >= 0){
		if(wParam == WM_LBUTTONDOWN && g_hookActive){
			return 1;
		}
		if(wParam == WM_LBUTTONUP && g_hookActive){
			gColorPickDialog->StartColorPickerTimer();
			UnhookWindowsHookEx(g_hook);
			g_hookActive = false;
			return 1;
		}
	}
	return CallNextHookEx(NULL,nCode,wParam,lParam);
}
#else
static void getColorCallback(float r, float g, float b)
{
    gColorPickDialog->UpdateColor(Color(r,g,b));
}
#endif

Bool SliderSubDialog::Command(Int32 id,const BaseContainer &msg)
{
	Int32 val;
	String str;
	Float rVal[4];
	Color col;
	int numberType;
	GetInt32(m_numberRangeCombo,numberType);
    switch (id)
    {
	case IDC_R:
	case IDC_G:
	case IDC_B:
		for(int i=0;i<3;i++){
			//GetFloat(RGBeditNumber[i],rVal[i]);
			rVal[i] = ReadNumber(m_RGBSlider + i,RGBeditNumber[i], numberType); 
		}
		UpdateColor(Color(rVal[0],rVal[1],rVal[2]).SetSource(COLOR_SOURCE_RGB));
		break;
    case IDC_C:
    case IDC_M:
    case IDC_Y:
    case IDC_K:
            for(int i=0;i<4;i++){
                //GetFloat(CMYKeditNumber[i],rVal[i]);
				rVal[i] = ReadNumber(m_CMYKSlider + i,CMYKeditNumber[i], numberType);
            }
            UpdateColor(Color(rVal[0],rVal[1],rVal[2],rVal[3]).SetSource(COLOR_SOURCE_CMYK));
            break;
    case IDC_H:
    case IDC_S:
    case IDC_V:
        for(int i=0;i<3;i++){
            rVal[i] = ReadNumber(m_HSVSlider + i,HSVeditNumber[i], numberType); 
            //GetFloat(HSVeditNumber[i],rVal[i]);
        }
        UpdateColor(Color(rVal[0],rVal[1],rVal[2],rVal[3]).SetSource(COLOR_SOURCE_WHEEL));
        break;
    case IDC_L:
    case IDC_A:
    case IDC_BB:
        for(int i=0;i<3;i++){
            GetFloat(LABeditNumber[i],rVal[i]);
        }
        UpdateColor(Color(rVal[0],rVal[1],rVal[2],rVal[3]).SetSource(COLOR_SOURCE_LAB));
        break;
	case IDC_RGBICC:
		GetInt32(iccRGBCombo,val);
		ChangeRGBSliderProfile(val);
		break;
	case IDC_CMYKICC:
		GetInt32(iccCMYKCombo,val);
		ChangeCMYKSliderProfile(val);
		break;
    case IDC_HSVICC:
        GetInt32(iccHSVCombo, val);
        Color::SetWheelProfile(val,TRUE);
        SpecialEventAdd(COLORPICKER_ID,-1,-1);
        break;
	case IDC_HEXTEXT:
		GetString(m_hexText,str);
		col.SetSource(COLOR_SOURCE_DISPLAY);
		if(col.FromString(str)){
			UpdateColor(col);
		}
		break;
	case IDC_NUMBERRANGE:
		GetString(m_hexText,str);
		col.SetSource(COLOR_SOURCE_DISPLAY);
		if(col.FromString(str)){
			UpdateColor(col);
		}
		break;
	case IDC_SCREEN_PICK_BUTTTON:
		{
            gColorPickDialog = this;
#ifdef _WINDOWS
			g_hookActive = true;
			g_hook = SetWindowsHookEx(WH_MOUSE_LL,MouseHookCallback,NULL,0);
#else
            char dylibPath[1024];
            String dir = GeGetPluginPath().GetString();
            dir += String("/osx/libColorPickerDynamic.dylib");
            dir.GetCString(dylibPath,1024);
            startColorPickFromScreen(dylibPath,&getColorCallback);
#endif
			break;
		}
	}
    return GeDialog::Command(id,msg);
}

Bool SliderSubDialog::CoreMessage(Int32 id,const BaseContainer &msg)
{
    switch (id) {
        case COLORPICKER_ID:
            // HSV mode has been changed
            ChangeHSVProfile();
            break;
        default:
            break;
    }
    return GeDialog::CoreMessage(id,msg);
}

void SliderSubDialog::Timer(const BaseContainer &msg)
{
#ifdef _WINDOWS
	// Abuse timer to handle picking of colors from screen
		POINT p;
		COLORREF color;
		HDC hDC;
		Bool b;
		hDC = GetDC(NULL);
		if(hDC != NULL){
			b = GetCursorPos(&p);
			if(b){
				color = GetPixel(hDC,p.x,p.y);
				if(color != CLR_INVALID){
					ReleaseDC(GetDesktopWindow(),hDC);
					UpdateColor(Color(GetRValue(color)/255.0,GetGValue(color)/255.0,GetBValue(color)/255.0).SetSource(COLOR_SOURCE_DISPLAY));
				}
			}
		}
#endif
	StopColorPickerTimer();
}

void SliderSubDialog::FindICCProfiles(){
	const GeDynamicArray<vnColorProfile> &RGBProfiles = Color::getRGBProfiles();
	const GeDynamicArray<vnColorProfile> &CMYKProfiles = Color::getCMYKProfiles();
	const GeDynamicArray<vnColorProfile> &spotProfiles = Color::getSpotProfiles();

	BaseContainer RGBbc, CMYKbc, HSVbc;

	for(int i=0;i<RGBProfiles.GetCount();i++){
		RGBbc.SetString(i,RGBProfiles[i].m_name);
	}
	for(int i=0;i<CMYKProfiles.GetCount();i++){
		CMYKbc.SetString(i,CMYKProfiles[i].m_name);
	}
    
    HSVbc.SetString(WHEEL_TYPE_HSV,"HSV");
    HSVbc.SetString(WHEEL_TYPE_HSB,"HSL");
    HSVbc.SetString(WHEEL_TYPE_LCH,"LCH");
    
	AddChildren(iccRGBCombo,RGBbc);
	SetInt32(iccRGBCombo,0);
	Enable(iccRGBCombo,TRUE);

	AddChildren(iccCMYKCombo,CMYKbc);
	SetInt32(iccCMYKCombo,0);
	Enable(iccCMYKCombo,TRUE);
    
    AddChildren(iccHSVCombo, HSVbc);
    SetInt32(iccHSVCombo, Color::m_wheelType);
    Enable(iccHSVCombo, TRUE);

	ChangeRGBSliderProfile(0);
}

void SliderSubDialog::UpdateColor(Color color){
	m_parent->UpdateColor(color);
}

Float SliderSubDialog::ReadNumber(ColorSlider *slider, C4DGadget *gadget, int type){
	Float val;
	Float maximum;
	switch(type){
	case 0:
		maximum = 255.0;
		break;
	case 1:
		maximum = 1.0;
		break;
	case 2:
		maximum = 100.0;
		break;
	}
	GetFloat(gadget,val);
	return (val/maximum)*(slider->m_valueMax - slider->m_valueMin) + slider->m_valueMin;
}

void SliderSubDialog::UpdateNumber(ColorSlider *slider, C4DGadget *gadget, float value, int type)
{
	float maximum, step;
	switch(type){
	case 0:
		maximum = 255.0;
		step = 1.0;
		break;
	case 1:
		maximum = 1.0;
		step = 0.1;
		break;
	case 2:
		maximum = 100.0;
		step = 0.1;
		break;
	}
	float val = (value - slider->m_valueMin)/(slider->m_valueMax - slider->m_valueMin)*maximum;
	if(step == 1.0){
		val = (float)((int)(val + 0.5));
	}
	SetFloat(gadget,val,0.0,maximum,step,FORMAT_FLOAT);
}

void SliderSubDialog::UpdateColorFromParent(Color color){
    Color display = color.Convert(COLOR_SOURCE_DISPLAY);
	Color wheel = color.Convert(COLOR_SOURCE_WHEEL);
	Color RGB = color.Convert(COLOR_SOURCE_RGB);
    Color LAB = color.Convert(COLOR_SOURCE_LAB);
    Color CMYK = color.Convert(COLOR_SOURCE_CMYK);
    ClampColor(display);
    ClampColor(wheel);
    ClampColor(RGB);
	int numberType;
	GetInt32(m_numberRangeCombo,numberType);
	for(Int32 i=0;i<3;i++){
		m_RGBSlider[i].UpdateColor(RGB);
		UpdateNumber(m_RGBSlider + i, RGBeditNumber[i],RGB[i],numberType);
	}
	for(Int32 i=0;i<4;i++){
		m_CMYKSlider[i].UpdateColor(CMYK);
		UpdateNumber(m_CMYKSlider + i, CMYKeditNumber[i],CMYK[i],numberType);
	}
    for(Int32 i=0;i<3;i++){
        m_HSVSlider[i].UpdateColor(wheel);
		UpdateNumber(m_HSVSlider + i, HSVeditNumber[i],wheel[i],numberType);
    }
    for(Int32 i=0;i<3;i++){
        m_LABSlider[i].UpdateColor(LAB);
        SetFloat(LABeditNumber[i], LAB[i]);
    }
	String s;
	display.ToString(s);
	SetString(m_hexText,s);
}

void SliderSubDialog::ChangeRGBSliderProfile(Int32 index)
{
    Color::SetRGBProfile(index,TRUE);
    if(!Color::IsRGBProfileOk()){
        Color::SetRGBProfile(0,TRUE);
    }
    Color col;
    String str;
    GetString(m_hexText,str);
    col.SetSource(COLOR_SOURCE_DISPLAY);
    if(col.FromString(str)){
        UpdateColor(col);
    }
}

void SliderSubDialog::ChangeCMYKSliderProfile(Int32 index)
{
    Color::SetCMYKProfile(index,TRUE);
    Color col;
    String str;
    GetString(m_hexText,str);
    col.SetSource(COLOR_SOURCE_DISPLAY);
    if(col.FromString(str)){
        UpdateColor(col);
    }
}

void SliderSubDialog::ChangeHSVProfile()
{
    SetInt32(iccHSVCombo, Color::m_wheelType);
    m_parent->UpdateWheel();
    Color col;
    String str;
    GetString(m_hexText,str);
    col.SetSource(COLOR_SOURCE_DISPLAY);
    if(col.FromString(str)){
        UpdateColor(col);
    }
}