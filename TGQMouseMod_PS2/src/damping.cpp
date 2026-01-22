#include "damping.hpp"
#include "utils.h"

Damper Damper_new(float initVal, float p, float maxErr) {
    return Damper {
        initVal,
        p,
        maxErr,
    };
}

float Damper_step(Damper& damper, float goal) {
    float err = goal - damper.val;
    
    // Apply damping
    damper.val += err * damper.k;

    // Hard clamp
    float newErr = goal - damper.val;
    if (newErr > damper.maxErr) {
        damper.val = goal - damper.maxErr;
    }
    else if (newErr < -damper.maxErr) {
        damper.val = goal + damper.maxErr;
    }

    return damper.val;
}

float Damper_step_wrapToPi(Damper& damper, float goal) {
    // Update state
    float err = wrapToPi(goal - damper.val);
    
    // Apply damper
    damper.val += err*damper.k;
    damper.val = wrapToPi(damper.val);

    // Hard clamp
    float newErr = wrapToPi(goal - damper.val);
    if (newErr > damper.maxErr) {
        damper.val = goal - damper.maxErr;
    }
    else if (newErr < -damper.maxErr) {
        damper.val = goal + damper.maxErr;
    }

    return damper.val;
}

float Damper_resetAt(Damper& damper, float goal) {
    damper.val = goal;

    return goal;
}
