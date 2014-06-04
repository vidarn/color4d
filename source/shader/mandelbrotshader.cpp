/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// example for an easy implementation of a channel shader

#include "c4d.h"
#include "c4d_symbols.h"
#include "xmandelbrot.h"

#define CCOUNT 125

class MandelbrotData : public ShaderData
{
	public:
		LONG offset;	
		Bool object_access;
		Vector *colors;
		Real threadRatio;
		String warpPattern;
		String warpOffset;
	public:
		virtual Bool Init		(GeListNode *node);
		virtual	Vector Output(BaseShader *chn, ChannelData *cd);

		virtual	INITRENDERRESULT InitRender(BaseShader *sh, const InitRenderStruct &irs);
		virtual	void FreeRender(BaseShader *sh);

		static NodeData *Alloc(void) { return gNew MandelbrotData; }
};

Bool MandelbrotData::Init(GeListNode *node)
{
	BaseContainer *data = ((BaseShader*)node)->GetDataInstance();
	data->SetLong(MANDELBROTSHADER_COLOROFFSET,100);
	data->SetBool(MANDELBROTSHADER_OBJECTACCESS,FALSE);
	data->SetReal(MANDELBROTSHADER_THREADRATIO,.3);
	data->SetString(WEAVESHADER_WARPPATTERN,"oou");
	data->SetString(WEAVESHADER_WARPOFFSET,"0");
	colors=NULL;
	return TRUE;
}

INITRENDERRESULT MandelbrotData::InitRender(BaseShader *sh, const InitRenderStruct &irs)
{
	BaseContainer *data = sh->GetDataInstance();

	colors = GeAllocType(Vector,CCOUNT);
	if (!colors) return INITRENDERRESULT_OUTOFMEMORY;

	LONG i,r,g,b;
	for (r=g=b=0,i=0;i<CCOUNT;i+=1)
	{
		colors[i]=irs.TransformColor(Vector(r/4.0,g/4.0,b/4.0));
		r+=1;
		if(r>4)
		{
			r=0;
			g+=1;
			if(g>4)
			{
				g=0;
				b+=1;
				if(b>4) b=0;
			}
		}
	}

	offset = data->GetLong(MANDELBROTSHADER_COLOROFFSET);
	object_access = data->GetBool(MANDELBROTSHADER_OBJECTACCESS);
	threadRatio = data->GetReal(MANDELBROTSHADER_THREADRATIO);
	warpPattern = data->GetString(WEAVESHADER_WARPPATTERN);
	warpOffset = data->GetString(WEAVESHADER_WARPOFFSET);

	GePrint("Warp Offset: ");
	GePrint(warpOffset);

	return INITRENDERRESULT_OK;
}

void MandelbrotData::FreeRender(BaseShader *sh)
{
	GeFree(colors);
}

static LONG Calcpix(Real r_min, Real i_min, Real border, LONG depth)
{
	LONG z;
	Real rz,iz,rq,iq;

	rz=r_min;
	iz=i_min;
	z=0;

	do
	{
		iq=iz*iz;
		rq=rz*rz;
		if((rq+iq)>border) return z;
		iz=((rz*iz)*2.0)+i_min;
		rz=rq-iq+r_min;
	} while (++z<depth);

	return z;
}

Vector MandelbrotData::Output(BaseShader *chn, ChannelData *cd)
{
	/*Real px=(cd->p.x*4.5)-2.5;
	Real py=(cd->p.y*3.0)-1.5;

	LONG i=Calcpix(px,py,5.0,50);
	Vector col = colors[(i+offset)%CCOUNT];

	if (cd->vd && object_access)
	{
		Real r,s,t;
		
		cd->vd->GetRS(cd->vd->lhit,cd->vd->p,&r,&s);

		t=1.0-(r+s);
		r=FMin(r,s);
		r=FMin(r,t);
		r=Smoothstep(RCO 0.0,RCO 0.3,r);

		col *= r + (1.0-r)*0.5*(Turbulence(cd->vd->p.ToRV()*0.1,4.0,FALSE)+1.0);
	}
	return col;*/
	Vector ret;
	Real numThreads = RCO 40.f;
	Real weftPosition = cd->p.x*numThreads;
	Real warpPosition = cd->p.y*numThreads;
	ULONG weftIndex = Floor(weftPosition);
	ULONG warpIndex = Floor(warpPosition);
	Real fu = weftPosition - weftIndex;
	Real fv = warpPosition - warpIndex;



	INT offset = 0;
	if(warpOffset.GetLength() > 0){
		offset = warpOffset[warpIndex % warpOffset.GetLength()] - '0';
	}
		
	INT i = (weftIndex + offset) % warpPattern.GetLength();
	Bool warpOver = warpPattern[i] == 'o';

	Bool hitWeft = fu < threadRatio;
	Bool hitWarp = fv < threadRatio;

	if(hitWeft || hitWarp){
		if(hitWarp && (!hitWeft || warpOver)){
			ret = Vector(RCO 1.f, RCO 0.f, RCO 0.f);
		}
		else{
			ret = Vector(RCO 1.f);
		}
	}
	else{
		ret = Vector(RCO 0.f);
	}
	
	return ret;
}

// be sure to use a unique ID obtained from www.plugincafe.com
#define ID_MANDELBROT	1001162

Bool RegisterMandelbrot(void)
{
	return RegisterShaderPlugin(ID_MANDELBROT,GeLoadString(IDS_MANDELBROT),0,MandelbrotData::Alloc,"Xmandelbrot",0);
}

