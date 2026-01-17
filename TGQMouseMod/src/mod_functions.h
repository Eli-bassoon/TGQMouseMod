#pragma once

// Global variables
__declspec(dllexport) extern char* cameraPtr;
__declspec(dllexport) extern char* frogPtr;
__declspec(dllexport) extern float cameraFocusY;
__declspec(dllexport) extern float frogNewYaw;
__declspec(dllexport) extern int queuedSpin;

// Helper functions
float clamp(float x, float min, float max);

float wrapToPi(float x);

bool castRay(float dist, float pitch, float yaw);

bool castGridRays(float dist);

void bumpCamera();

// Injected functions
void __declspec(dllexport) __stdcall storeMousePos();

int __declspec(dllexport) __stdcall storeMouseClick();

void __declspec(dllexport) __stdcall orbitCamera(char* camera);

void __declspec(dllexport)  __stdcall freelookCamera();

void __declspec(dllexport) __stdcall testApplyQueuedSpin();