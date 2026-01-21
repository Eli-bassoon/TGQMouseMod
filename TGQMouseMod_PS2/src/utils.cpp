#include <utils.h>

float fmin(float x, float y) {
    return (x < y) ? x : y;
}

float fmax(float x, float y) {
    return (x > y) ? x : y;
}

float fabs(float x) {
    return (x < 0) ? -x : x;
}

float clamp(float x, float min, float max) {
    return fmin(fmax(x, min), max);
}

float wrapToPi(float x) {
    if (x > PI) {
        x -= 2 * PI;
    }
    else if (x < -PI) {
        x += 2 * PI;
    }
    return x;
}

// Debug function that prints a float as an integer
int floatFixed(float x) {
    return static_cast<int>(x * 100);
}