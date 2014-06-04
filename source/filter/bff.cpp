/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// BFF (=basic file format) image loader and saver example

#include "c4d.h"
#include "c4d_symbols.h"

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_BFFLOADER 1000983
#define ID_BFFSAVER  1000957

class BFFLoaderData : public BitmapLoaderData
{
	public:
		virtual Bool Identify(const Filename &name, UCHAR *probe, LONG size);
		virtual IMAGERESULT Load(const Filename &name, BaseBitmap *bm, LONG frame);
		virtual LONG GetSaver(void) { return ID_BFFSAVER; }
};

class BFFSaverData : public BitmapSaverData
{
	public:
		virtual IMAGERESULT Save(const Filename &name, BaseBitmap *bm, BaseContainer *data, SAVEBIT savebits);
		virtual LONG GetMaxResolution(Bool layers) { return MAXLONGl; }
};

Bool BFFLoaderData::Identify(const Filename &name, UCHAR *probe, LONG size)
{
	ULONG *p=(ULONG*)probe,v1=p[0];					 
	lMotor(&v1); 
	return v1==0x42464600;
}

IMAGERESULT BFFLoaderData::Load(const Filename &name, BaseBitmap *bm, LONG frame)
{
	LONG			y,bw,bh,header;
	Bool			ok    = TRUE;
	UCHAR			*line = NULL;
	AutoAlloc<BaseFile>	file;
	if (!file) return IMAGERESULT_OUTOFMEMORY;

	if (!file->Open(name,FILEOPEN_READ,FILEDIALOG_NONE,BYTEORDER_MOTOROLA)) return IMAGERESULT_FILEERROR;

	file->ReadLong(&header);
	file->ReadLong(&bw);
	file->ReadLong(&bh);

	line = GeAllocType(UCHAR,3*bw);

	if (bm->Init(bw,bh,24)!=IMAGERESULT_OK || !line) 
	{ 
		GeFree(line); 
		return IMAGERESULT_OUTOFMEMORY; 
	}

	for (y=0; y<bh && ok; y++)
	{
		file->ReadBytes(line,bw*3);
		ok = bm->SetPixelCnt(0,y,bw,line,COLORBYTES_RGB,COLORMODE_RGB,PIXELCNT_0);
	}

	ok = ok && file->GetError()==FILEERROR_NONE;
	GeFree(line);
	
	return ok ? IMAGERESULT_OK : IMAGERESULT_FILEERROR;
}

IMAGERESULT BFFSaverData::Save(const Filename &name, BaseBitmap *bm, BaseContainer *data, SAVEBIT savebits)
{
	LONG			y,bw,bh;
	Bool			ok    = TRUE;
	UCHAR			*line = NULL;
	AutoAlloc<BaseFile> file;
	if (!file) return IMAGERESULT_OUTOFMEMORY;

	if (!file->Open(name,FILEOPEN_WRITE,FILEDIALOG_NONE,BYTEORDER_MOTOROLA)) return IMAGERESULT_FILEERROR;

	bw=bm->GetBw();
	bh=bm->GetBh();
	
	file->WriteLong(0x42464600);
	file->WriteLong(bw);
	file->WriteLong(bh);

	line = GeAllocType(UCHAR,3*bw);
	if (!line) 
	{ 
		GeFree(line); 
		return IMAGERESULT_OUTOFMEMORY; 
	}

	for (y=0; y<bh; y++)
	{
		bm->GetPixelCnt(0,y,bw,line,COLORBYTES_RGB,COLORMODE_RGB,PIXELCNT_0);
		file->WriteBytes(line,bw*3);
	}

	ok=file->GetError()==FILEERROR_NONE;
	GeFree(line);
	
	return ok?IMAGERESULT_OK:IMAGERESULT_FILEERROR;
}

Bool RegisterBFF(void)
{
	String name=GeLoadString(IDS_BFF);
	if (!RegisterBitmapLoaderPlugin(ID_BFFLOADER,name,0,gNew BFFLoaderData)) return FALSE;
	if (!RegisterBitmapSaverPlugin(ID_BFFSAVER,name,PLUGINFLAG_BITMAPSAVER_SUPPORT_8BIT,gNew BFFSaverData,"bff")) return FALSE;
	return TRUE;
}

