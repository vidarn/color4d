#pragma once
#include "c4d.h"

class Color;

Float Wrap(Float val, Float min, Float max);

Bool VerifyColor(Vector col);

void ClampColor(Vector &col);

void ClampColor(Color &col);