#pragma once

#include "kcC.h"

// Global variables used in assembly
__declspec(dllexport) extern char* cameraPtr;
__declspec(dllexport) extern char* frogPtr;
__declspec(dllexport) extern float cameraFocusY;
__declspec(dllexport) extern float frogNewYaw;
__declspec(dllexport) extern int queuedSpin;

#define CAMERA_FROG_POS_OFFSET 0x268 // Offset from cameraPtr to the frog position
#define CAMERA_PID_OFFSET 0x178 // Offset from cameraPtr to the PIDs for position and rotation
#define CAMERA_DELTA_TIME_OFFSET 0x2f8 // Offset from frogPtr to the yaw PID
#define FROG_TURN_PID_OFFSET 0x88 // Offset from frogPtr to the yaw PID

// Injected functions
void __declspec(dllexport) __stdcall orbitCamera(char* camera);

void __declspec(dllexport)  __stdcall freelookCamera();

void __declspec(dllexport) __stdcall applyQueuedSpin();

// Variables from the original game
extern float& analogYInputNorm;
extern int& pressedKeysPtr;
extern __declspec(dllexport) char*& gameSystemPtr; // Reference to pointer of game system

// Functions from the original game
typedef int(__thiscall* raycastFnSig)(char* gameSystem, kcVector4* pPoint, kcVector4* pDirection, float* pDist, int _, unsigned int mask, unsigned int ignore, _kcSurfaceInfo* pReturnInfo);
extern __declspec(dllexport) raycastFnSig raycast;