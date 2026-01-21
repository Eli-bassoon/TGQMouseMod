#pragma once

#include <symbols.h>
#include <utils.h>

// Here we define a damping struct
// This is a struct, not a class, because weird initialization stuff happens and resets whatever values I normally give it
// I create uninitialized structs in the main file, then initialize them once inside a function
struct Damper {
    float val;
    float k;
    float maxErr;
};

Damper Damper_new(float initVal, float k, float maxErr);

float Damper_step(Damper& damper, float goal);

float Damper_step_wrapToPi(Damper& damper, float goal);

float Damper_resetAt(Damper& damper, float goal);
