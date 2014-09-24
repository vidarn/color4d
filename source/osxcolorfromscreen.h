#pragma once

#ifndef _WINDOWS
void startColorPickFromScreen(const char* path, void (*getColorCallback)(float r, float g, float b));
#endif