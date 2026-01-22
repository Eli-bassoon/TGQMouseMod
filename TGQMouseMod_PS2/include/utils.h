#pragma once

#define PI 3.14159265358979323846f

// Only define these if using PS2, otherwise use functions from cmath
#ifdef PS2
float fmin(float x, float y);

float fmax(float x, float y);

float fabs(float x);
#endif

float clamp(float x, float min, float max);

float wrapToPi(float x);

int floatFixed(float x);
