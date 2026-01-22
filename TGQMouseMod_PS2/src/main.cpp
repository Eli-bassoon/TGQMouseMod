// PS2 specific includes
#ifdef PS2
#include "symbols.h"

// PC specific includes
#else
#include <cmath>
#include "symbols_pc.h"
#include "mouse_utils.h"

#endif

// General includes
#include <utility>
#include "types.h"
#include "kcC.h"
#include "utils.h"
#include "decl_macros.h"
#include "damping.hpp"

char* cameraPtr;
char* frogPtr;

// Free look
EXPORT_VAR float sensitivity = 1.0f;

EXPORT_VAR float freelookScaleX = 0.0090f;
EXPORT_VAR float freelookScaleY = -0.0077f;

EXPORT_VAR float freelookMaxSpeedX = 0.25f;
EXPORT_VAR float freelookMaxSpeedY = 0.25f;

// Orbit
EXPORT_VAR float orbitScaleX = 0.012f;
EXPORT_VAR float orbitScaleY = -0.010f;

EXPORT_VAR float orbitMaxSpeedX = 0.5f;
EXPORT_VAR float orbitMaxSpeedY = 0.2f;

EXPORT_VAR float orbitYOffset = 0.2f;

EXPORT_VAR float orbitPitchClampOffset = 0.1f;

EXPORT_VAR float minOrbitDist = 0.2f;
EXPORT_VAR float maxOrbitDist = 4.0f;
EXPORT_VAR float orbitDist = maxOrbitDist;

EXPORT_VAR float orbitBumpDist = 0.15f;
EXPORT_VAR float orbitHalfFovW = 0.15f;
EXPORT_VAR float orbitHalfFovH = 0.07f;

Damper cameraYawDamper;
Damper cameraPitchDamper;
EXPORT_VAR float cameraYaw = 0.0f; // These are separate variables so we can link to them in assembly more easily
EXPORT_VAR float cameraPitch = 0.5f;

EXPORT_VAR float frogNewYaw = 0.0f;
EXPORT_VAR float frogNewPitch = 0.0f;
EXPORT_VAR int queuedSpin = 0;

Damper cameraFocusYDamper;
EXPORT_VAR float cameraFocusY = 0.0f;

// Raycasting
EXPORT_VAR int castSteps = 10;

// The expected values of surface info from debugging the original raycast code
_kcSurfaceInfo surfaceInfo = { 0, 0x1F94A278, 0, 0 };

#ifdef PS2
// Get the camera deltas from the analog stick

std::pair<float, float> getCameraDeltas() {
    return {analogStickRX-127, analogStickRY-127};
}

#else
// Variables from the original game
float& analogYInputNorm = *reinterpret_cast<float*>(0x004DC470);
int& pressedKeysPtr = *reinterpret_cast<int*>(0x004DC44C);
__declspec(dllexport) char*& gameSystemPtr = *reinterpret_cast<char**>(0x004e4528); // Reference to pointer of game system

// Functions from the original game
typedef int(__thiscall* raycastFnSig)(char* gameSystem, kcVector4* pPoint, kcVector4* pDirection, float* pDist, int _, unsigned int mask, unsigned int ignore, _kcSurfaceInfo* pReturnInfo);
__declspec(dllexport) raycastFnSig raycast = reinterpret_cast<raycastFnSig>(0x00416750);

#endif

// Raycast from Frogger to the camera
bool castRay(float dist, float pitch, float yaw) {
    kcVector4 origin = *reinterpret_cast<kcVector4*>(cameraPtr + CAMERA_FROG_POS_OFFSET);
    kcVector4 normal = {
        sinf(pitch) * cosf(yaw),
        cosf(pitch),
        sinf(pitch) * sinf(yaw)
    };

    int hit = raycast(
        gameSystemPtr,
        &origin,
        &normal,
        &dist,
#ifndef PS2
        0, // The raycast function signature seems to be different between PS2 and PC, with PC always expecting a zero before the mask
#endif
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

// Orbiting camera control with mouse
EXPORT_FUNC(void) orbitCamera(char* camera) {
    // Initialize state only one time
    ONCE {
        cameraFocusYDamper = Damper_new(0, 0.15f, 2.0f);
        cameraYawDamper = Damper_new(0, 0.9f, 0.2f);
        cameraPitchDamper = Damper_new(0.5f, 0.85f, 0.2f);
    }
    
    cameraPtr = camera;
    float deltaTime = *reinterpret_cast<float*>(cameraPtr + CAMERA_DELTA_TIME_OFFSET);

    // Add mouse coordinates and clamp
    auto cameraDeltas = getCameraDeltas();
    cameraYaw += clamp(cameraDeltas.first * sensitivity * orbitScaleX * deltaTime, -orbitMaxSpeedX, orbitMaxSpeedX);
    cameraPitch += clamp(cameraDeltas.second * sensitivity * orbitScaleY * deltaTime, -orbitMaxSpeedY, orbitMaxSpeedY);

    cameraYaw = wrapToPi(cameraYaw);
    cameraPitch = clamp(cameraPitch, orbitPitchClampOffset, PI - orbitPitchClampOffset);

    Damper_step_wrapToPi(cameraYawDamper, cameraYaw);
    Damper_step_wrapToPi(cameraPitchDamper, cameraPitch);

    frogNewYaw = wrapToPi(-cameraYawDamper.val - PI / 2);
    frogNewPitch = clamp(-cameraPitchDamper.val + PI / 2, -1.55f, 1.55f);

    // Get Frogger's position
    auto frogPos = reinterpret_cast<kcVector4*>(cameraPtr + CAMERA_FROG_POS_OFFSET);
    float frogX = frogPos->x;
    float frogY = frogPos->y;
    float frogZ = frogPos->z;

    // Do damping on the Y coordinate, aiming to be an offset above Frogger
    float goalY = frogY + orbitYOffset;
    cameraFocusY = Damper_step(cameraFocusYDamper, goalY);

    // We are already clamping to a range, but if we go even further than that range, directly set the focus position
    // This helps avoid jarring transitions when switching levels
    if (fabs(cameraFocusY - goalY) > 5.0f) {
        Damper_resetAt(cameraFocusYDamper, goalY);
    }

    // Offset in spherical coordinates
    float dx = orbitDist * sinf(cameraPitchDamper.val) * cosf(cameraYawDamper.val);
    float dy = orbitDist * cosf(cameraPitchDamper.val);
    float dz = orbitDist * sinf(cameraPitchDamper.val) * sinf(cameraYawDamper.val);

    // The camera's position is stored in a set of PID structs for x y z pitch yaw roll
    auto cameraPidPtr = reinterpret_cast<cameraPid*>(cameraPtr + CAMERA_PID_OFFSET);
    cameraPidPtr->x.x = frogX + dx;
    cameraPidPtr->y.x = cameraFocusY + dy;
    cameraPidPtr->z.x = frogZ + dz;

    // Finally, bump the camera if we would clip out of the world
    bumpCamera();
}

// First-person freelook control with mouse
EXPORT_FUNC(void) freelookCamera() {
    float deltaTime = *reinterpret_cast<float*>(cameraPtr + CAMERA_DELTA_TIME_OFFSET);

    // Add mouse coordinates and clamp
    auto cameraDeltas = getCameraDeltas();
    cameraYaw += clamp(cameraDeltas.first * sensitivity * freelookScaleX * deltaTime, -freelookMaxSpeedX, freelookMaxSpeedX);
    cameraPitch += clamp(cameraDeltas.second * sensitivity * freelookScaleY * deltaTime, -freelookMaxSpeedY, freelookMaxSpeedY);

    cameraYaw = wrapToPi(cameraYaw);
    cameraPitch = clamp(cameraPitch, orbitPitchClampOffset, PI - orbitPitchClampOffset);

    // Don't do any damping when in first person
    Damper_resetAt(cameraYawDamper, cameraYaw);
    Damper_resetAt(cameraPitchDamper, cameraPitch);

    // Convert to freelook coordinates
    frogNewYaw = wrapToPi(-cameraYawDamper.val - PI / 2);
    frogNewPitch = clamp(-cameraPitchDamper.val + PI / 2, -1.55f, 1.55f);

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
EXPORT_FUNC(void) applyQueuedSpin() {
    if (queuedSpin) {
        // Set the PID parameters for Frogger's yaw rotation, so he turns away from the camera
        auto frogYawPid = reinterpret_cast<_kcPid*>(frogPtr + FROG_TURN_PID_OFFSET);
        frogYawPid->x = frogNewYaw;
        frogYawPid->eq = frogNewYaw;
        frogYawPid->v = 0;
        frogYawPid->i = 0;

        // Set the analog input to forward, forcing Frogger to move forward a slight amount and rotate towards the camera
        analogYInputNorm = 1.0f;

        --queuedSpin;
    }
}