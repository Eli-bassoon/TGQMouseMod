#ifndef SYMBOLS_H
#define SYMBOLS_H
#include <types.h>
#include <kcC.h>

extern int analogueStickRX;
extern int analogueStickRY;
extern float analogueStickLYNorm;
extern char* gameSystem;

char* cameraPtr;
char* frogPtr;

// External functions
extern "C"
{
    extern float sinf(float x);
    extern float cosf(float x);
    extern int printf(char* format, ...);
    extern int _print(char* s);
    extern int raycast(char* gameSystem, kcVector4* pPoint, kcVector4* pDirection, float* pDist, unsigned int mask, unsigned int ignore, _kcSurfaceInfo* pReturnInfo);
}

#endif //SYMBOLS_H