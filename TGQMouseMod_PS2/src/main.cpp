#include <types.h>
#include <symbols.h>
#include "kcC.h"

#define PI 3.14159265358979323846f

#define CAMERA_FROG_POS_OFFSET 0x290 // Offset from cameraPtr to the frog position
#define CAMERA_PID_OFFSET 0x1a0 // Offset from cameraPtr to the PIDs for position and rotation
#define FROG_TURN_PID_OFFSET 0x88 // Offset from frogPtr to the yaw PID

// Free look
float freelookScaleX = 50.0f;
float freelookScaleY = -60.0f;

float freelookMaxSpeedX = 0.2f;
float freelookMaxSpeedY = 0.2f;

// Orbit
float orbitScaleX = 50.0f;
float orbitScaleY = -55.0f;

float orbitMaxSpeedX = 0.3f;
float orbitMaxSpeedY = 0.2f;

float orbitYOffset = 0.2f;

float orbitPitchClampOffset = 0.1f;

float minOrbitDist = 0.2f;
float maxOrbitDist = 4.0f; // Was originally 4
float orbitDist = maxOrbitDist;

float orbitBumpDist = 0.15f;
float orbitHalfFovW = 0.15f;
float orbitHalfFovH = 0.07f;

float cameraYaw = 0.0f;
float cameraPitch = 0.5f;

float frogNewYaw = 0.0f;
float frogNewPitch = 0.0f;
int queuedSpin = 0;

float cameraFocusY = 0.0f;
float cameraYPrevError = 0.0f;
float cameraYPidP = 0.1f;
float cameraYPidD = 0.0f;

// Raycasting
int castSteps = 10;

// The expected values of surface info from debugging the original raycast code
_kcSurfaceInfo surfaceInfo = { 0, 0x1F94A278, 0, 0 };

float fmin(float x, float y) {
    return (x < y) ? x : y;
}

float fmax(float x, float y) {
    return (x > y) ? x : y;
}

// Helper functions
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

// Raycast utility
bool castRay(float dist, float pitch, float yaw) {
    kcVector4 origin = *reinterpret_cast<kcVector4*>(cameraPtr + CAMERA_FROG_POS_OFFSET);
    kcVector4 normal = {
        sinf(pitch) * cosf(yaw),
        cosf(pitch),
        sinf(pitch) * sinf(yaw)
    };

    int hit = raycast(
        gameSystem,
        &origin,
        &normal,
        &dist,
        0x11001,
        0x2000000,
        &surfaceInfo
    );
    return hit;
}

bool castGridRays(float dist) {
    // Cast a 3x3 grid of rays to check if we are clipping through a wall
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (castRay(dist, cameraPitch + dy * orbitHalfFovH, cameraYaw + dx * orbitHalfFovW)) return true;
        }
    }
    return false;
}

// Move the camera to not clip through walls
void bumpCamera() {
    // The distance between successive cast steps
    float stepDist = (maxOrbitDist - minOrbitDist) / castSteps;

    // Shortcut: See if the max distance isn't hitting anything
    if (!castGridRays(maxOrbitDist + orbitBumpDist)) {
        orbitDist = maxOrbitDist;
        return;
    }

    // Bisection to find distance
    float lo = minOrbitDist;
    float hi = maxOrbitDist;
    float mid = maxOrbitDist;
    for (int step = 0; step < castSteps; ++step) {
        mid = (hi + lo) / 2;
        // If we intersect, look in earlier half
        if (castGridRays(mid + orbitBumpDist)) {
            hi = mid;
        }
        // Otherwise, look in later half
        else {
            lo = mid;
        }
    }
    orbitDist = mid;
}

float prevYaw = 0;
float prevPitch = 0;

// Orbiting camera control with mouse
extern "C" void orbitCamera(char* camera, float deltaTime) {
    cameraPtr = camera;

    prevYaw = cameraYaw;
    prevPitch = cameraPitch;

    // Add mouse coordinates and clamp
    cameraYaw += clamp(static_cast<float>(analogueStickRX - 127) / orbitScaleX, -orbitMaxSpeedX, orbitMaxSpeedX);
    cameraPitch += clamp(static_cast<float>(analogueStickRY - 127) / orbitScaleY, -orbitMaxSpeedY, orbitMaxSpeedY);

    cameraYaw = wrapToPi(prevYaw * 0.2f + cameraYaw * 0.8f);
    cameraPitch = clamp(prevPitch * 0.2f + cameraPitch * 0.8f, orbitPitchClampOffset, PI - orbitPitchClampOffset);

    frogNewYaw = wrapToPi(-cameraYaw - PI / 2);
    frogNewPitch = clamp(-cameraPitch + PI / 2, -1.55f, 1.55f);

    // Get Frogger's position
    auto frogPos = reinterpret_cast<kcVector4*>(cameraPtr + CAMERA_FROG_POS_OFFSET);
    float frogX = frogPos->x;
    float frogY = frogPos->y;
    float frogZ = frogPos->z;

    // Do PID control on the Y coordinate, aiming to be an offset above Frogger
    float cameraYError = (frogY + orbitYOffset) - cameraFocusY;
    // Teleport for large errors
    if (cameraYError > 5.0f) {
        cameraYError = 0;
        cameraYPrevError = 0;
        cameraFocusY = frogY + orbitYOffset;
    }
    // Do PD for small errors
    else {
        float dError = cameraYError - cameraYPrevError;
        float extra = cameraYError * cameraYPidP - dError * cameraYPidD;
        cameraFocusY += extra;
        cameraYPrevError = cameraYError;
    }

    // Offset in spherical coordinates
    float dx = orbitDist * sinf(cameraPitch) * cosf(cameraYaw);
    float dy = orbitDist * cosf(cameraPitch);
    float dz = orbitDist * sinf(cameraPitch) * sinf(cameraYaw);

    // The camera struct aligns the x y z PID structs next to each other
    auto cameraPidPtr = reinterpret_cast<cameraPid*>(cameraPtr + CAMERA_PID_OFFSET);
    cameraPidPtr->x.x = frogX + dx;
    cameraPidPtr->y.x = cameraFocusY + dy;
    cameraPidPtr->z.x = frogZ + dz;

    // Finally, bump the camera if we would clip out of the world
    bumpCamera();
}

// First-person freelook control with mouse
extern "C" void freelookCamera() {
    prevYaw = cameraYaw;
    prevPitch = cameraPitch;

    // Add mouse coordinates and clamp
    cameraYaw += clamp(static_cast<float>(analogueStickRX - 127) / freelookScaleX, -freelookMaxSpeedX, freelookMaxSpeedX);
    cameraPitch += clamp(static_cast<float>(analogueStickRY - 127) / freelookScaleY, -freelookMaxSpeedY, freelookMaxSpeedY);

    cameraYaw = wrapToPi(prevYaw * 0.1f + cameraYaw * 0.9f);
    cameraPitch = clamp(prevPitch * 0.1f + cameraPitch * 0.9f, orbitPitchClampOffset, PI - orbitPitchClampOffset);

    // Convert to freelook coordinates
    frogNewYaw = wrapToPi(-cameraYaw - PI / 2);
    frogNewPitch = clamp(-cameraPitch + PI / 2, -1.55f, 1.55f);

    auto cameraPidPtr = reinterpret_cast<cameraPid*>(cameraPtr + CAMERA_PID_OFFSET);
    cameraPidPtr->pitch.x = frogNewPitch;
    cameraPidPtr->yaw.x = frogNewYaw;

    // Set the PID parameters for Frogger's yaw rotation, so he moves with the camera
    auto frogYawPid = reinterpret_cast<_kcPid*>(frogPtr + FROG_TURN_PID_OFFSET);
    frogYawPid->x = frogNewYaw;
    frogYawPid->eq = frogNewYaw;
    frogYawPid->v = 0;
    frogYawPid->i = 0;
}

// Due to the camera being weird and always resetting the rotation, we have to simulate moving forward for one frame to make it stick
extern "C" void testApplyQueuedSpin() {
    if (queuedSpin) {
        // Set the PID parameters for Frogger's yaw rotation, so he turns away from the camera
        auto frogYawPid = reinterpret_cast<_kcPid*>(frogPtr + FROG_TURN_PID_OFFSET);
        frogYawPid->x = frogNewYaw;
        frogYawPid->eq = frogNewYaw;
        frogYawPid->v = 0;
        frogYawPid->i = 0;

        // Set the analogue input to forward, forcing Frogger to move forward a slight amount and rotate towards the camera
        analogueStickLYNorm = 1.0f;

        --queuedSpin;
    }
}