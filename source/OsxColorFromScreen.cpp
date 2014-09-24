#include "osxcolorfromscreen.h"

#ifndef _WINDOWS

#include <dlfcn.h>
#include <stdio.h>

void startColorPickFromScreen(const char* path, void (*getColorCallback)(float r, float g, float b))
{
    printf(path);
    void *dylibPtr = dlopen(path,RTLD_LAZY);
    if(dylibPtr != 0){
        void *fnPtr = dlsym(dylibPtr, "startColorPick");
        if(fnPtr != 0){
            void (*startColorPick)(void (*)(float, float, float)) = (void (*)(void (*)(float, float, float)))fnPtr;
            if(startColorPick != 0){
                startColorPick(getColorCallback);
            }
        }
    }
}

#endif