#ifndef SYMBOLS_H
#define SYMBOLS_H
#include "types.h"
#include "kcC.h"

extern int analogStickRX;
extern int analogStickRY;
extern float analogYInputNorm;
extern char* gameSystemPtr;

extern char* cameraPtr;
extern char* frogPtr;

// Struct offsets
#define CAMERA_FROG_POS_OFFSET 0x290 // Offset from cameraPtr to the frog position
#define CAMERA_PID_OFFSET 0x1a0 // Offset from cameraPtr to the PIDs for position and rotation
#define CAMERA_DELTA_TIME_OFFSET 0x324 // Offset from cameraPtr to deltaTime
#define FROG_TURN_PID_OFFSET 0x88 // Offset from frogPtr to the yaw PID

// External functions
extern "C"
{
    // From C standard library
    extern float sinf(float x);
    extern float cosf(float x);
    extern int printf(char* format, ...);
    
    // From game
    extern int raycast(char* gameSystem, kcVector4* pPoint, kcVector4* pDirection, float* pDist, unsigned int mask, unsigned int ignore, _kcSurfaceInfo* pReturnInfo);

    // Syscall print
    extern int _print(char* s);
}

#endif //SYMBOLS_H