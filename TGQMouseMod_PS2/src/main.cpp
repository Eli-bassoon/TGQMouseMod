#include <types.h>
#include <symbols.h>
#include <kcC.h>
#include <utils.h>
#include <damping.hpp>

#define CAMERA_FROG_POS_OFFSET 0x290 // Offset from cameraPtr to the frog position
#define CAMERA_PID_OFFSET 0x1a0 // Offset from cameraPtr to the PIDs for position and rotation
#define CAMERA_DELTA_TIME_OFFSET 0x324 // Offset from cameraPtr to deltaTime
#define FROG_TURN_PID_OFFSET 0x88 // Offset from frogPtr to the yaw PID

char* cameraPtr;
char* frogPtr;

// Free look
float sensitivity = 1.0f;

float freelookScaleX = 0.0090f;
float freelookScaleY = -0.0077f;

float freelookMaxSpeedX = 0.25f;
float freelookMaxSpeedY = 0.25f;

// Orbit
float orbitScaleX = 0.012f;
float orbitScaleY = -0.010f;

float orbitMaxSpeedX = 0.5f;
float orbitMaxSpeedY = 0.2f;

float orbitYOffset = 0.2f;

float orbitPitchClampOffset = 0.1f;

float minOrbitDist = 0.2f;
float maxOrbitDist = 4.0f;
float orbitDist = maxOrbitDist;

float orbitBumpDist = 0.15f;
float orbitHalfFovW = 0.15f;
float orbitHalfFovH = 0.07f;

Damper cameraYawDamper;
Damper cameraPitchDamper;
float cameraYaw = 0.0f; // These are separate variables so we can link to them in assembly more easily
float cameraPitch = 0.5f;

float frogNewYaw = 0.0f;
float frogNewPitch = 0.0f;
int queuedSpin = 0;

Damper cameraFocusYDamper;
float cameraFocusY = 0.0f;

// Raycasting
int castSteps = 10;

// The expected values of surface info from debugging the original raycast code
_kcSurfaceInfo surfaceInfo = { 0, 0x1F94A278, 0, 0 };

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

// Orbiting camera control with mouse
extern "C" void orbitCamera(char* camera) {
    // Initialize state only one time
    ONCE {
        cameraFocusYDamper = Damper_new(0, 0.15f, 2.0f);
        cameraYawDamper = Damper_new(0, 0.9f, 0.2f);
        cameraPitchDamper = Damper_new(0.5f, 0.85f, 0.2f);
    }
    
    cameraPtr = camera;
    float deltaTime = *reinterpret_cast<float*>(cameraPtr + CAMERA_DELTA_TIME_OFFSET);

    // Add mouse coordinates and clamp
    cameraYaw += clamp(static_cast<float>(analogueStickRX - 127) * sensitivity * orbitScaleX * deltaTime, -orbitMaxSpeedX, orbitMaxSpeedX);
    cameraPitch += clamp(static_cast<float>(analogueStickRY - 127) * sensitivity * orbitScaleY * deltaTime, -orbitMaxSpeedY, orbitMaxSpeedY);

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
extern "C" void freelookCamera() {
    float deltaTime = *reinterpret_cast<float*>(cameraPtr + CAMERA_DELTA_TIME_OFFSET);

    // Add mouse coordinates and clamp
    cameraYaw += clamp(static_cast<float>(analogueStickRX - 127) * sensitivity * freelookScaleX * deltaTime, -freelookMaxSpeedX, freelookMaxSpeedX);
    cameraPitch += clamp(static_cast<float>(analogueStickRY - 127) * sensitivity * freelookScaleY * deltaTime, -freelookMaxSpeedY, freelookMaxSpeedY);

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