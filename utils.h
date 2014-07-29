#pragma once
#include "c4d.h"

Float Wrap(Float val, Float min, Float max);

Bool VerifyColor(Vector col);

void ClampColor(Vector &col);