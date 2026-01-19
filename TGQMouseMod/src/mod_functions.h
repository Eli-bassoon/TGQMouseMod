#pragma once

// Global variables
__declspec(dllexport) extern char* cameraPtr;
__declspec(dllexport) extern char* frogPtr;
__declspec(dllexport) extern float cameraFocusY;
__declspec(dllexport) extern float frogNewYaw;
__declspec(dllexport) extern int queuedSpin;

#define CAMERA_FROG_POS_OFFSET 0x268 // Offset from cameraPtr to the frog position
#define CAMERA_PID_OFFSET 0x178 // Offset from cameraPtr to the PIDs for position and rotation
#define CAMERA_DELTA_TIME_OFFSET 0x2f8 // Offset from frogPtr to the yaw PID
#define FROG_TURN_PID_OFFSET 0x88 // Offset from frogPtr to the yaw PID

// Helper functions
float clamp(float x, float min, float max);

float wrapToPi(float x);

bool castRay(float dist, float pitch, float yaw);

bool castGridRays(float dist);

void bumpCamera();

// Injected functions
void __declspec(dllexport) __stdcall storeMousePos();

int __declspec(dllexport) __stdcall storeMouseClick();

void __declspec(dllexport) __stdcall orbitCamera(char* camera, float deltaTime);

void __declspec(dllexport)  __stdcall freelookCamera(float deltaTime);

void __declspec(dllexport) __stdcall applyQueuedSpin();