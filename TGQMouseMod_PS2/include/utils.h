#pragma once

#define PI 3.14159265358979323846f

float fmin(float x, float y);

float fmax(float x, float y);

float fabs(float x);

float clamp(float x, float min, float max);

float wrapToPi(float x);

int floatFixed(float x);
