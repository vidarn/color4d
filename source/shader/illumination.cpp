/////////////////////////////////////////////////////////////
// CINEMA 4D SDK                                           //
/////////////////////////////////////////////////////////////
// (c) 1989-2004 MAXON Computer GmbH, all rights reserved  //
/////////////////////////////////////////////////////////////

// original source code of Illuminance1..Illuminance3
// The API versions are faster (no calling overhead and
// various speedups) but if you want to introduce new 
// illumination models this source is the base for further
// experiments

#include "c4d.h"

inline void LowClip(Vector *v)
{
	if (v->x<0.0) v->x=0.0;
	if (v->y<0.0) v->y=0.0;
	if (v->z<0.0) v->z=0.0;
}

void ShIlluminance1(VolumeData *sd, Vector *diffuse, Vector *specular, Real exponent)
{
	RayLight		*lp;
	LONG				i;
	Vector			col;
	LVector			lv;
	Real				cosa,cosb;
	Bool				nodif,nospec,calc_specular = exponent>0.0;

	*specular = 0.0;

	if (sd->calc_illum)
		*diffuse = 0.0;
	else
	{
		*diffuse = 1.0;
		return;
	}

	for (i=sd->GetLightCount()-1; i>=0; i--)
	{
		lp = sd->GetLight(i);
		if (!sd->IlluminateSurfacePoint(lp,&col,&lv,sd->p,sd->bumpn,sd->n,sd->orign,sd->ray->v,sd->calc_shadow?ILLUMINATEFLAGS_SHADOW:ILLUMINATEFLAGS_0,sd->lhit,sd->raybits,TRUE)) continue;
		if (lp->ambient) { *diffuse += col; continue; }

		nodif=nospec=FALSE;
		if (lp->lr.object) CalcRestrictionInc(&lp->lr,sd->op,nodif,nospec);

		if (lp->arealight)
		{
			// tube and area lights are difficult to calculate
			// if you want to combine area lights with your own lighting models
			// you completely need to rebuild the area light shapes!
			// for standard models use LightCalcSpecial
	
			Vector dif,spc;
			sd->CalcArea(lp,nodif,nospec,exponent,sd->ray->v,sd->p,sd->bumpn,sd->orign,sd->raybits,&dif,&spc);

			*diffuse  += dif^col;
			*specular += spc^col;
		}
		else 
		{
			cosa=lv*sd->bumpn;
			if (!(lp->nodiffuse || nodif) && sd->cosc*cosa>=0.0) 
			{
				if (lp->contrast!=0.0)
					*diffuse += Pow(Abs(cosa),Exp(-lp->contrast))*col;
				else
					*diffuse += Abs(cosa)*col;
			}

			if (calc_specular && !(lp->nospecular || nospec))
			{
				cosb=sd->ray->v * (lv - sd->bumpn*(2.0 * cosa));
				if (cosb<=0.0) continue;

				*specular += Pow(cosb,exponent)*col;
			}
		}
	}

	LowClip(diffuse); // there can be subtractive light sources
	LowClip(specular);
}

void ShIlluminance2(VolumeData *sd, Vector *diffuse, Vector *specular, Real exponent, const LVector &p, const LVector &n, const LVector &bumpn, const LVector &rayv, ILLUMINATEFLAGS calc_shadow, RAYBIT raybits)
{
  // shouldn't be used any longer - this routine does not handle area lights well
	RayLight	*lp;
	LONG			i;
	Vector		col;
	LVector		lv;
	Real			cosa,cosb,cosc;
	Bool			calc_specular = exponent>0.0;

	*specular = 0.0;
	*diffuse  = 0.0;

	cosc = -rayv*n;

	for (i=sd->GetLightCount()-1; i>=0; i--)
	{
		lp = sd->GetLight(i);
		if (!sd->IlluminateSurfacePoint(lp,&col,&lv,p,bumpn,bumpn,n,rayv,calc_shadow,RayHitID(),raybits,TRUE)) continue;

		cosa=lv*bumpn;
		if (cosc*cosa>=0.0) 
			*diffuse  += Abs(cosa)*col;

		if (!calc_specular) continue;

		cosb=rayv * (lv - bumpn*(2.0 * cosa));
		if (cosb<=0.0) continue;

		*specular += Pow(cosb,exponent)*col;
	}

	LowClip(diffuse); // there can be subtractive light sources
	LowClip(specular);
}

Vector ShIlluminance3(VolumeData *sd, const LVector &p, ILLUMINATEFLAGS calc_shadow, RAYBIT raybits)
{
	RayLight	*lp;
	LONG			i;
	Vector		col,lv,sum=0.0;

	for (i=sd->GetLightCount()-1; i>=0; i--)
	{
		lp = sd->GetLight(i);
		if (!sd->IlluminateAnyPoint(lp,&col,NULL,p,calc_shadow,raybits)) continue;
		sum += col;
	}

	LowClip(&sum); // there can be subtractive light sources

	return sum;
}
