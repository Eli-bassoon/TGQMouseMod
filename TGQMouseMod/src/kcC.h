#pragma once

// Struct definitions from the decompiled source code, used by functions we call in the original code

struct kcVector4
{
    float x;
    float y;
    float z;
    float w;
};

struct _kcSurfaceInfo
{
	unsigned int mFlags;
	unsigned int mSurfaceID;
	unsigned int mReserved2;
	unsigned int mReserved1;
};

struct _kcPid
{
	float x;
	float eq;
	float v;
	float i;
	float a;
	float b;
	float c;
	float reserved;
};

// This struct doesn't exist exactly like this in the code but has the right layout for the camera's position and rotation PID controllers
struct cameraPid
{
	_kcPid x;
	_kcPid y;
	_kcPid z;
	_kcPid pitch;
	_kcPid yaw;
	_kcPid roll;
};