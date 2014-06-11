#pragma once
#include "c4d.h"

Real Wrap(Real val, Real min, Real max);

Bool VerifyColor(Vector col);

void ClampColor(Vector &col);