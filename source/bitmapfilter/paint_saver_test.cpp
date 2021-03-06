// this example source code saves out the selected BP texture 
// in all combinations (bitdepth, rgb, grey) of all registered 
// bitmap filters

#include "c4d_commanddata.h"
#include "c4d_resource.h"
#include "c4d_general.h"
#include "c4d_painter.h"
#include "c4d_filterdata.h"
#include "lib_description.h"
#include "c4d_includes.h"



#define ID_BITMAP_SAVER_TEST 200000109



class BitmapSaverTest : public CommandData
{
  INSTANCEOF(BitmapSaverTest, CommandData)

public:
  virtual Bool Execute(BaseDocument* doc);

  static BitmapSaverTest* Alloc() { return gNew BitmapSaverTest; }
};


void CreateImageSaverList(BasePlugin *bp, AtomArray *array)
{
	for (;bp; bp=bp->GetNext())
	{
		if (bp->GetPluginType()==PLUGINTYPE_BITMAPSAVER)
		{
			array->Append(bp);
		}
		if (bp->GetDown())
			CreateImageSaverList(bp->GetDown(),array);				
	}
}

void ExportImage(PaintTexture *tex,BitmapSaverPlugin *bp, COLORMODE colormode,LONG needbits, SAVEBIT savebits,const String &append)
{
	if (!(bp->GetInfo()&needbits)) return;

	LONG saveformat = bp->GetID();

	String suffix;
	LONG alpha;
	bp->BmGetDetails(&alpha,&suffix);

//	if (suffix!="tif") return;

	GeData d_filename;
	tex->GetParameter(DescLevel(ID_PAINTTEXTURE_FILENAME),d_filename,DESCFLAGS_GET_0);

	Filename fn = d_filename.GetFilename();

	fn.ClearSuffix();

	if (bp->GetName().ToUpper().FindFirst("QUICKTIME",NULL,0))
		fn.SetFile(fn.GetFileString()+String("_QUICK_")+suffix+String("_")+append);
	else
		fn.SetFile(fn.GetFileString()+String("_C4D_")+suffix+String("_")+append);
	fn.SetSuffix(suffix);

	PaintTexture *copy = (PaintTexture*)tex->GetClone(COPYFLAGS_0,NULL);
	if (!copy) return;

	copy->SetColorMode(colormode,FALSE);

	switch (needbits)
	{
		case PLUGINFLAG_BITMAPSAVER_SUPPORT_8BIT:
		case PLUGINFLAG_BITMAPSAVER_SUPPORT_16BIT:
		case PLUGINFLAG_BITMAPSAVER_SUPPORT_32BIT:
			{
				AutoAlloc<BaseBitmap> merge;
				if (copy->ReCalc(NULL,0,0,copy->GetBw(),copy->GetBh(),merge,RECALC_NOGRID|RECALC_INITBMP,0))
				{
					BaseContainer data;
					if (merge->Save(fn,saveformat,&data,savebits|SAVEBIT_ALPHA)!=IMAGERESULT_OK)
					{
						GeOutString("Error saving image \""+append+"\"",GEMB_OK);
					}
				}
			}
			break;

		case PLUGINFLAG_BITMAPSAVER_SUPPORT_8BIT_LAYERS:
		case PLUGINFLAG_BITMAPSAVER_SUPPORT_16BIT_LAYERS:
		case PLUGINFLAG_BITMAPSAVER_SUPPORT_32BIT_LAYERS:
			{
				copy->SetParameter(DescLevel(ID_PAINTTEXTURE_FILENAME),fn,DESCFLAGS_SET_0);
				copy->SetParameter(DescLevel(ID_PAINTTEXTURE_SAVEFORMAT),saveformat,DESCFLAGS_SET_0);

				BaseContainer bc;
				bc.SetLong(PAINTER_SAVETEXTURE_FLAGS,PAINTER_SAVEBIT_SAVECOPY);
				SendPainterCommand(PAINTER_SAVETEXTURE,NULL,copy,&bc);
			}
			break;
	}
	SendPainterCommand(PAINTER_FORCECLOSETEXTURE,NULL,copy,NULL);
}

Bool BitmapSaverTest::Execute(BaseDocument *doc)
{
/*
	AutoAlloc<BaseBitmap> bmp;
	bmp->Init(100,100,24);
	BaseBitmap *a1 = bmp->AddChannel(TRUE,FALSE);
	BaseBitmap *a2 = bmp->AddChannel(FALSE,FALSE);
	bmp->Clear(255,128,0);
	if (a1)
	{
		LONG y; UCHAR buf[] = { 128 };
		for (y=0;y<100;y++)
			a1->SetPixelCnt(0,y,100,buf,0,COLORMODE_GRAY,0);
	}
	if (a2)
	{
		LONG y; UCHAR buf[] = { 255 };
		for (y=0;y<100;y++)
			a2->SetPixelCnt(0,y,100,buf,0,COLORMODE_GRAY,0);
	}
	bmp->Save(Filename("G:\\3D\\BP3 test stuff\\format_conversion_test\\test.tif"),FILTER_TIF,NULL,SAVEBIT_32BITCHANNELS|SAVEBIT_GREYSCALE|SAVEBIT_ALPHA);
//	bmp->Save(Filename("G:\\3D\\BP3 test stuff\\format_conversion_test\\test.tif"),FILTER_TIF,NULL,SAVEBIT_16BITCHANNELS|SAVEBIT_ALPHA);
//return TRUE;
*/
	PaintTexture *tex = PaintTexture::GetSelectedTexture();
	if (!tex)
	{
		GeOutString("No Texture Selected",GEMB_OK);
		return FALSE;
	}

	BasePlugin *bp = GetFirstPlugin();
	AutoAlloc<AtomArray> array;
	CreateImageSaverList(bp,array);
	LONG i;
	for (i=0;i<array->GetCount();i++)
	{
		BitmapSaverPlugin *bp = (BitmapSaverPlugin*)array->GetIndex(i);

		ExportImage(tex,bp,COLORMODE_RGB ,PLUGINFLAG_BITMAPSAVER_SUPPORT_8BIT        ,SAVEBIT_0                               ,"_8Bit_rgb_flat"  );
		ExportImage(tex,bp,COLORMODE_RGBw,PLUGINFLAG_BITMAPSAVER_SUPPORT_16BIT       ,SAVEBIT_16BITCHANNELS                   ,"16Bit_rgb_flat"  );
		ExportImage(tex,bp,COLORMODE_RGBf,PLUGINFLAG_BITMAPSAVER_SUPPORT_32BIT       ,SAVEBIT_32BITCHANNELS                   ,"32Bit_rgb_flat"  );
		ExportImage(tex,bp,COLORMODE_RGB ,PLUGINFLAG_BITMAPSAVER_SUPPORT_8BIT_LAYERS ,SAVEBIT_MULTILAYER                      ,"_8Bit_rgb_layers" );
		ExportImage(tex,bp,COLORMODE_RGBw,PLUGINFLAG_BITMAPSAVER_SUPPORT_16BIT_LAYERS,SAVEBIT_MULTILAYER|SAVEBIT_16BITCHANNELS,"16Bit_rgb_layers");
		ExportImage(tex,bp,COLORMODE_RGBf,PLUGINFLAG_BITMAPSAVER_SUPPORT_32BIT_LAYERS,SAVEBIT_MULTILAYER|SAVEBIT_32BITCHANNELS,"32Bit_rgb_layers");

		ExportImage(tex,bp,COLORMODE_GRAY,PLUGINFLAG_BITMAPSAVER_SUPPORT_8BIT        ,SAVEBIT_GREYSCALE                                         ,"_8Bit_grey_flat"  );
		ExportImage(tex,bp,COLORMODE_GRAYw,PLUGINFLAG_BITMAPSAVER_SUPPORT_16BIT       ,SAVEBIT_GREYSCALE|SAVEBIT_16BITCHANNELS                   ,"16Bit_grey_flat"  );
		ExportImage(tex,bp,COLORMODE_GRAYf,PLUGINFLAG_BITMAPSAVER_SUPPORT_32BIT       ,SAVEBIT_GREYSCALE|SAVEBIT_32BITCHANNELS                   ,"32Bit_grey_flat"  );
		ExportImage(tex,bp,COLORMODE_GRAY,PLUGINFLAG_BITMAPSAVER_SUPPORT_8BIT_LAYERS ,SAVEBIT_GREYSCALE|SAVEBIT_MULTILAYER                      ,"_8Bit_grey_layers" );
		ExportImage(tex,bp,COLORMODE_GRAYw,PLUGINFLAG_BITMAPSAVER_SUPPORT_16BIT_LAYERS,SAVEBIT_GREYSCALE|SAVEBIT_MULTILAYER|SAVEBIT_16BITCHANNELS,"16Bit_grey_layers");
		ExportImage(tex,bp,COLORMODE_GRAYf,PLUGINFLAG_BITMAPSAVER_SUPPORT_32BIT_LAYERS,SAVEBIT_GREYSCALE|SAVEBIT_MULTILAYER|SAVEBIT_32BITCHANNELS,"32Bit_grey_layers");
	}

  return TRUE;
}






Bool RegisterPainterSaveTest()
{
	#ifdef _DEBUG
	  return RegisterCommandPlugin(ID_BITMAP_SAVER_TEST, String("C++ SDK - BitmapSaverTest"), 0, NULL, String("BitmapSaverTest"), BitmapSaverTest::Alloc());
	#else
		return TRUE;
	#endif
}