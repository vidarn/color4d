#include "utils.h"
#include "color.h"

Float Wrap(Float val, Float min, Float max)
{
	Float step = max-min;
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
		col[i] = ClampValue(col[i],0.0,1.0);
	}
}

void ClampColor(Color &col)
{
    for(int i=0;i<4;i++){
        col[i] = ClampValue(col[i], 0.0, 1.0);
    }
}