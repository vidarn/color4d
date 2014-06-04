#ifndef __GRADIENTGADGET_H
#define __GRADIENTGADGET_H

#include "ge_math.h"

#define MAXGRADIENT	20

class BaseBitmap;
class GeUserArea;

struct SDKGradient
{
	Vector	col;
	Real		pos;
	LONG		id;
};

class SDKGradientGadget
{
	private:
		LONG				iw,ih,xmin,active,*count,*interpol,maxgrad;
		LONG				dragx,dragy,dragid;
		Vector			dragcol;
		SDKGradient	*g;
		GeUserArea	*ua;

		Real YtoP(LONG y);
		LONG PtoY(Real pos);
		void GetBoxPosition(LONG num, LONG *x, LONG *y);
		LONG InsertBox(Vector col, Real per, LONG id);
		LONG FindID(void);
		void RemoveBox(LONG num);

	public:
		BaseBitmap *col;

		SDKGradientGadget(void);
		~SDKGradientGadget(void);

		void Init(GeUserArea *a_ua, SDKGradient *a_g, LONG *a_count, LONG *a_interpol, LONG a_maxgrad);
		Bool InitDim(LONG x, LONG y);

		Bool MouseDown(LONG x,LONG y,Bool dbl);
		void MouseDrag(LONG x,LONG y);

		void SetPosition(Real per);
		Bool GetPosition(Real *per);

		void CalcImage(void);
};												 

Vector CalcGradientMix(const Vector &g1, const Vector &g2, Real per, LONG interpol);

#endif
