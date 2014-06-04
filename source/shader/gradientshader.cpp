/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example for a complex channel shader with custom areas
// and animated preview
#include "c4d.h"
#include "c4d_symbols.h"
#include "xsdkgradient.h"

struct GradientData
{
	Bool			cycle;
	LONG			mode;	
	Real			angle;
	Vector		c[4];
	Real			sa,ca;

	Real			turbulence,octaves,scale,freq;
	Bool			absolute;

	Gradient*	gradient;
};

class SDKGradientClass : public ShaderData
{
	public:
		GradientData gdata;
	public:
		virtual Bool Init(GeListNode *node);
		virtual	Vector Output(BaseShader *sh, ChannelData *cd);
		virtual	INITRENDERRESULT InitRender(BaseShader *sh, const InitRenderStruct &irs);
		virtual	void FreeRender(BaseShader *sh);

		static NodeData *Alloc(void) { return gNew SDKGradientClass; }
};

Bool SDKGradientClass::Init(GeListNode *node)
{
	BaseContainer *data = ((BaseShader*)node)->GetDataInstance();

	AutoAlloc<Gradient> gradient;
	if (!gradient) return FALSE;

	GradientKnot k1,k2;
	k1.col =Vector(0.0,0.0,1.0);
	k1.pos =0.0;

	k2.col =1.0;
	k2.pos =1.0;

	gradient->InsertKnot(k1);
	gradient->InsertKnot(k2);

	data->SetData(SDKGRADIENTSHADER_COLOR,GeData(CUSTOMDATATYPE_GRADIENT,gradient));
	data->SetBool(SDKGRADIENTSHADER_CYCLE,FALSE);
	data->SetLong(SDKGRADIENTSHADER_MODE,0);
	data->SetReal(SDKGRADIENTSHADER_ANGLE,0.0);

	data->SetReal(SDKGRADIENTSHADER_TURBULENCE,0.0);
	data->SetReal(SDKGRADIENTSHADER_OCTAVES,5.0);
	data->SetReal(SDKGRADIENTSHADER_SCALE,1.0);
	data->SetReal(SDKGRADIENTSHADER_FREQ,1.0);
	data->SetBool(SDKGRADIENTSHADER_ABSOLUTE,FALSE);
	
	return TRUE;
}

INITRENDERRESULT SDKGradientClass::InitRender(BaseShader *sh, const InitRenderStruct &irs)
{
	BaseContainer *dat = sh->GetDataInstance();

	gdata.mode			 = dat->GetLong(SDKGRADIENTSHADER_MODE); 
	gdata.angle			 = dat->GetReal(SDKGRADIENTSHADER_ANGLE); 
	gdata.cycle			 = dat->GetBool(SDKGRADIENTSHADER_CYCLE);
	gdata.turbulence = dat->GetReal(SDKGRADIENTSHADER_TURBULENCE);
	gdata.octaves    = dat->GetReal(SDKGRADIENTSHADER_OCTAVES);
	gdata.scale      = dat->GetReal(SDKGRADIENTSHADER_SCALE);
	gdata.freq       = dat->GetReal(SDKGRADIENTSHADER_FREQ);
	gdata.absolute   = dat->GetBool(SDKGRADIENTSHADER_ABSOLUTE);
	gdata.gradient	 = (Gradient*)dat->GetCustomDataType(SDKGRADIENTSHADER_COLOR,CUSTOMDATATYPE_GRADIENT); 
	if (!gdata.gradient || !gdata.gradient->InitRender(irs)) return INITRENDERRESULT_OUTOFMEMORY;

	gdata.sa=Sin(gdata.angle);
	gdata.ca=Cos(gdata.angle);

	LONG i;
	GradientKnot *k;

	for (i=0; i<4; i++)
	{
		gdata.c[i]=0.0;
		k = gdata.gradient->GetRenderKnot(i); 
		if (k) gdata.c[i]=k->col; 
	}

	return INITRENDERRESULT_OK;
}

void SDKGradientClass::FreeRender(BaseShader *sh)
{
	if (gdata.gradient) gdata.gradient->FreeRender();
	gdata.gradient = NULL;
}

Vector SDKGradientClass::Output(BaseShader *sh, ChannelData *sd)
{
	Vector p=sd->p;
	Real	 r=0.0,angle,xx,yy;

	if (gdata.turbulence>0.0)
	{
		Vector	res;
		Real		scl=5.0*gdata.scale,tt=sd->t*gdata.freq*0.3;

		res = Vector(Turbulence(p*scl,tt,gdata.octaves,TRUE),Turbulence((p+Vector(0.34,13.0,2.43))*scl,tt,gdata.octaves,TRUE),0.0);

		if (gdata.absolute)
		{
			p.x  = Mix(p.x,res.x,gdata.turbulence);
			p.y  = Mix(p.y,res.y,gdata.turbulence);
		}
		else
		{
			p.x += (res.x-0.5)*gdata.turbulence;
			p.y += (res.y-0.5)*gdata.turbulence;
		}
	}

	// rotation
	p.x -= 0.5;
	p.y -= 0.5;

	xx = gdata.ca*p.x-gdata.sa*p.y + 0.5;
	yy = gdata.sa*p.x+gdata.ca*p.y + 0.5;

	p.x = xx;
	p.y = yy;

	if (gdata.mode<=SDKGRADIENTSHADER_MODE_CORNER && gdata.cycle && (sd->texflag&TEX_TILE))
	{
		if (sd->texflag & TEX_MIRROR)
		{
			p.x = Modulo(p.x,RCO 2.0);
			if (p.x>=1.0) p.x=2.0-p.x;

			p.y = Modulo(p.y,RCO 2.0);
			if (p.y>= 1.0) p.y=2.0-p.y;
		}
		else
		{
			p.x = Modulo(p.x, RCO 1.0);
			p.y = Modulo(p.y, RCO 1.0);
		}
	}

	switch (gdata.mode)
	{
		case SDKGRADIENTSHADER_MODE_U:
			r = p.x; 
			break;

		case SDKGRADIENTSHADER_MODE_V:
			r = 1.0-p.y; 
			break;

		case SDKGRADIENTSHADER_MODE_DIAGONAL:
			r = (p.x+p.y)*0.5; 
			break;

		case SDKGRADIENTSHADER_MODE_RADIAL:
			p.x-=0.5;
			p.y-=0.5;
			if (p.x==0.0) p.x=0.00001;

			angle = ATan(p.y/p.x);
			if (p.x<0.0) angle+=pi;
			if (angle<0.0) angle+=pi2;
			r = angle/pi2;
			break;

		case SDKGRADIENTSHADER_MODE_CIRCULAR:
			p.x-=0.5;
			p.y-=0.5;
			r = Sqrt(p.x*p.x+p.y*p.y)*2.0;
			break;

		case SDKGRADIENTSHADER_MODE_BOX:
			p.x = Abs(p.x - 0.5);
			p.y = Abs(p.y - 0.5);
			r   = FMax(p.x,p.y)*2.0;
			break;

		case SDKGRADIENTSHADER_MODE_STAR:
			p.x = Abs(p.x - 0.5)-0.5;
			p.y = Abs(p.y - 0.5)-0.5;
			r   = Sqrt(p.x*p.x+p.y*p.y) * 1.4142;
			break;

		case SDKGRADIENTSHADER_MODE_CORNER:
		{
			Real		cx;
			Vector	ca,cb;

			cx = FCut01(p.x);
			ca = Mix(gdata.c[0],gdata.c[1],cx);
			cb = Mix(gdata.c[2],gdata.c[3],cx);

			return Mix(ca,cb,FCut01(p.y));
			break;
		}
	}

	return gdata.gradient->CalcGradientPixel(FCut01(r));
}

Bool RegisterGradient(void)
{
	Filename fn = GeGetPluginPath()+"res"+"gradienttypes.tif";
	AutoAlloc<BaseBitmap> bmp;
	if (IMAGERESULT_OK!=bmp->Init(fn)) return FALSE;

	RegisterIcon(200000135,bmp,0*32,0,32,32,ICONFLAG_COPY);
	RegisterIcon(200000136,bmp,1*32,0,32,32,ICONFLAG_COPY);
	RegisterIcon(200000137,bmp,2*32,0,32,32,ICONFLAG_COPY);
	RegisterIcon(200000138,bmp,3*32,0,32,32,ICONFLAG_COPY);
	RegisterIcon(200000139,bmp,4*32,0,32,32,ICONFLAG_COPY);
	RegisterIcon(200000140,bmp,5*32,0,32,32,ICONFLAG_COPY);
	RegisterIcon(200000141,bmp,6*32,0,32,32,ICONFLAG_COPY);
	RegisterIcon(200000142,bmp,7*32,0,32,32,ICONFLAG_COPY);

	// be sure to use a unique ID obtained from www.plugincafe.com
	return RegisterShaderPlugin(1001161,GeLoadString(IDS_SDKGRADIENT),0,SDKGradientClass::Alloc,"Xsdkgradient",0);
}
