#include "utils.h"

Real Wrap(Real val, Real min, Real max)
{
	Real step = max-min;
	while(val < min){
		val += step;
	}
	while(val > max){
		val -= step;
	}
	return val;
}

Bool VerifyColor(Vector col)
{
	Bool result = TRUE;
	for(int i=0;i<3;i++){
		if(col[i] < 0.0 || col[i] > 1.0){
			result = FALSE;
			break;
		}
	}
	return result;
}

void ClampColor(Vector &col)
{
	for(int i=0;i<3;i++){
		col[i] = Clamp(0.0,1.0,col[i]);
	}
}