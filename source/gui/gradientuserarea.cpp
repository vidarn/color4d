#include "c4d.h"
#include "gradientuserarea.h"
#include "c4d_colors.h"

Vector CalcGradientMix(const Vector &g1, const Vector &g2, Real per, LONG interpol)
{
	switch (interpol)
	{
		default:
		case 0: return g1; break;
		case 1: return g1+(g2-g1)*per; break;
		case 2: return g1+(g2-g1)*Ln(per*1.7182819+1.0); break;
		case 3: return g1+(g2-g1)*(1.0-Ln((1.0-per)*1.7182819+1.0)); break;
		case 4: return g1+(g2-g1)*Smoothstep(RCO 0.0,RCO 1.0,per); break;
	}
}

static Vector CalcGradientPixel(Real y, SDKGradient *g, LONG count, LONG interpol)
{											 
	LONG	i;											 
	Real	delta;

	i=0;
	while (i+1<count && y>=g[i+1].pos) i++;

	if (i+1<count && y>=g[i].pos)
	{
		delta = g[i+1].pos-g[i].pos;
		if (delta==0.0) return g[i].col;
		return CalcGradientMix(g[i].col,g[i+1].col,(y-g[i].pos)/delta,interpol);
	}
	else if (i<count)
		return g[i].col;
	else
		return 0.0;
}

SDKGradientGadget::SDKGradientGadget(void)
{
	col = BaseBitmap::Alloc();
}

SDKGradientGadget::~SDKGradientGadget(void)
{
	BaseBitmap::Free(col);
}

#define BOXRAD 4 

void SDKGradientGadget::Init(GeUserArea *a_ua, SDKGradient *a_g, LONG *a_count, LONG *a_interpol, LONG a_maxgrad)
{
	ua				= a_ua;
	maxgrad   = a_maxgrad;
	g					= a_g;
	count			= a_count;
	interpol	= a_interpol;
	active    = *a_count-1;
}

Bool SDKGradientGadget::InitDim(LONG bw, LONG bh)
{	
	if (!col) return FALSE;
	iw=bw; ih=bh;
	xmin=iw-ih*3/2; 
	if (xmin<=50) xmin=50;
	if (col->Init(iw,ih,24)!=IMAGERESULT_OK) return FALSE;
	CalcImage();
	return TRUE;
}

void SDKGradientGadget::SetPosition(Real per)
{
	if (active<0 || active>=*count) return;
	per = FCut01(per);

	SDKGradient gg = g[active];
	RemoveBox(active);
	active=InsertBox(gg.col,per,gg.id);

	CalcImage();			
}

Bool SDKGradientGadget::GetPosition(Real *per)
{
	if (!count || active<0 || active>=*count) return FALSE;
	*per = g[active].pos;
	return TRUE;
}

Bool SDKGradientGadget::MouseDown(LONG x,LONG y, Bool dbl)
{
	if (x<0 || y<0 || x>=iw || y>=ih) return FALSE;

	LONG			i,xp,yp,num;
	Real			per;
	Bool			move=FALSE;

	for (i=0; i<*count; i++)
	{
		GetBoxPosition(i,&xp,&yp);
		if (x>=xp-BOXRAD && x<=xp+BOXRAD && y>=yp-BOXRAD && y<=yp+BOXRAD) break;
	}

	if (i<*count || (x>=xmin && y>=BOXRAD && y<ih-BOXRAD && *count+1<=maxgrad) && !dbl) 
	{
		move = i<*count;

		if (move || dbl)
		{
			dragcol = g[i].col;
			num			= i;
			GetBoxPosition(i,&xp,&yp);
		}
		else
		{
			per = YtoP(y);
			dragcol = CalcGradientPixel(per,g,*count,*interpol);
			num = InsertBox(dragcol,per,FindID());
			xp = x; yp = y;
			move=TRUE;
		}
		active=num;

		dragx=xp-x; 
		dragy=yp-y; 
		dragid = g[num].id;
	
		if (dbl)
		{
			GeChooseColor(&g[num].col,0);
			move=FALSE;
		}

		CalcImage();
	}

	return move;
}

void SDKGradientGadget::MouseDrag(LONG x,LONG y)
{
	x+=dragx;
	y+=dragy;

	Real per;
	if (active!=-1 && Abs(YtoP(y)-g[active].pos)<1.0/Real(ih-2*BOXRAD-1))
		return;

	per = Truncate(YtoP(y)*1000.0+0.5)*0.001;
	per = FCut01(per);
	if (active!=-1) RemoveBox(active);

	if (x>=0 && x<iw && y>=-25 && y<=ih+25)
		active=InsertBox(dragcol,per,dragid);
	else
		active=-1;
	
	CalcImage();
}

Real SDKGradientGadget::YtoP(LONG y)
{
	return 1.0-(y-BOXRAD)/Real(ih-2*BOXRAD-1);
}

LONG SDKGradientGadget::PtoY(Real pos)
{
	return BOXRAD+LONG((1.0-pos)*Real(ih-2*BOXRAD-1)+0.01);		
}

void SDKGradientGadget::GetBoxPosition(LONG num, LONG *x, LONG *y)
{
	LONG i,j,temp,yi,ys,mask,pos=0;
	UCHAR *field=NULL;
	LONG	*sort=NULL;

	field = GeAllocType(UCHAR,ih);
	if (!field) return;

	sort = GeAllocType(LONG,maxgrad);
	if (!sort) { GeFree(field); return; }
	
	*x = xmin-BOXRAD-6;
	*y = PtoY(g[num].pos);

	for (i=0; i<*count; i++)
		sort[i] = i;

	// bubble sort
	for (i=0; i<*count-1; i++)
	{
		for (j=i+1; j<*count; j++)
		{
			if (g[sort[i]].id>g[sort[j]].id)
			{
				temp = sort[i]; sort[i] = sort[j]; sort[j] = temp;
			}
		}
	}

	for (i=0; i<*count; i++)
	{
		j=sort[i];

		ys = PtoY(g[j].pos);
		
		mask = 0;
		for (yi=ys-BOXRAD; yi<=ys+BOXRAD; yi++)
			mask |= field[yi];

		if (!(mask&1)) 
			pos = 0;
		else if (!(mask&2))
			pos = 1;
		else if (!(mask&4)) 
			pos = 2;
		else if (!(mask&8))
			pos = 3;
		else 
			pos = 4;

		if (j==num) break;

		for (yi=ys-BOXRAD; yi<=ys+BOXRAD; yi++)
			field[yi] |= 1<<pos;
	}
	
	GeFree(sort);
	GeFree(field);

	*x -= (2*BOXRAD+1)*pos;
}

LONG SDKGradientGadget::FindID(void)
{
	LONG i,id;
	CHAR *used = GeAllocType(CHAR,maxgrad);
	if (!used) return 0;

	for (i=0; i<*count; i++)
		used[g[i].id] = TRUE;

	for (id=0; id<maxgrad; id++)
		if (!used[id])
			break;
	
	if (id>=maxgrad)  id=0; // safety check
	
	GeFree(used);
	return id;
}

LONG SDKGradientGadget::InsertBox(Vector col, Real per, LONG id)
{
	LONG i,num=0;
	
	while (num<*count && per>g[num].pos) num++;

	for (i=*count; i>=num+1; i--)
		g[i] = g[i-1];

	g[num].col = col;
	g[num].pos = per;
	g[num].id  = id;

	count[0]++;
	return num;
}

void SDKGradientGadget::RemoveBox(LONG num)
{
	LONG i;
	
	for (i=num; i<*count-1; i++)
		g[i] = g[i+1];

	count[0]--;
}

void SDKGradientGadget::CalcImage(void)
{
	if (!col) return;

	LONG		i,x,y,pass;
	UWORD		rr,gg,bb;
	Vector	v;

	ua->DrawSetPen(COLOR_BG);
	ua->FillBitmapBackground(col,0,0);
	
	for (pass=0; pass<3; pass++)
	{
		for (i=0; i<*count; i++)
		{
			if (pass==0)
			{
				GetBoxPosition(i,&x,&y);
				col->SetPen(0,0,0);
				if (x>=0)
					col->Line(x,y,xmin-1,y);
			}
			else 
			{
				if ((pass==2)!=(i==active)) continue;
				
				GetBoxPosition(i,&x,&y);

				rr = LONG(g[i].col.x*COLOR);
				gg = LONG(g[i].col.y*COLOR);
				bb = LONG(g[i].col.z*COLOR);

				if (pass==2) 
					col->SetPen(255,255,255);
				else
					col->SetPen(0,0,0);

				if (x-BOXRAD>0)
				{
					col->Line(x-BOXRAD,y+BOXRAD,x+BOXRAD,y+BOXRAD);
					col->Line(x+BOXRAD,y-BOXRAD,x+BOXRAD,y+BOXRAD);
					col->Line(x-BOXRAD,y-BOXRAD,x+BOXRAD,y-BOXRAD);
					col->Line(x-BOXRAD,y-BOXRAD,x-BOXRAD,y+BOXRAD);

					col->Clear(x-BOXRAD+1,y-BOXRAD+1,x+BOXRAD-1,y+BOXRAD-1,rr,gg,bb);
				}
			}
		}
	}

	for (y=BOXRAD; y<ih-BOXRAD; y++)
	{
		v = CalcGradientPixel(YtoP(y),g,*count,*interpol);
		rr = LONG(v.x*COLOR);
		gg = LONG(v.y*COLOR);
		bb = LONG(v.z*COLOR);
		col->SetPen(rr,gg,bb);
		col->Line(xmin,y,iw-1,y);
	}
}

