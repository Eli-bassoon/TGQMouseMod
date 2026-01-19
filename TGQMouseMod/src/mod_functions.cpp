#include "Windows.h"
#include <cmath>
#include "kcC.h"
#include "mod_functions.h"

#define PI 3.14159265358979323846f

__declspec(dllexport) int prevMouseX = 0;
__declspec(dllexport) int prevMouseY = 0;
__declspec(dllexport) int mouseX = 0;
__declspec(dllexport) int mouseY = 0;

__declspec(dllexport) int mouseDx = 0;
__declspec(dllexport) int mouseDy = 0;

__declspec(dllexport) bool captureCursor = true;

// Pointers to game memory
__declspec(dllexport) char* cameraPtr;
__declspec(dllexport) char* frogPtr;
float* analogueYInputPtr = reinterpret_cast<float*>(0x004DC470);
int* pressedKeysPtr = reinterpret_cast<int*>(0x004DC44C);
__declspec(dllexport) char** gameSystemPtrLoc = reinterpret_cast<char**>(0x004e4528); // Static address in the exe

// Free look
__declspec(dllexport) float freelookScaleX = 1200.0f;
__declspec(dllexport) float freelookScaleY = -900.0f;

__declspec(dllexport) float freelookMaxSpeedX = 0.15f;
__declspec(dllexport) float freelookMaxSpeedY = 0.1f;

// Orbit
__declspec(dllexport) float orbitScaleX = 900.0f;
__declspec(dllexport) float orbitScaleY = -700.0f;

__declspec(dllexport) float orbitMaxSpeedX = 0.3f;
__declspec(dllexport) float orbitMaxSpeedY = 0.2f;

__declspec(dllexport) float orbitPitchClampOffset = 0.1f;

__declspec(dllexport) float minOrbitDist = 0.2f;
__declspec(dllexport) float maxOrbitDist = 3.5f; // Was originally 4
__declspec(dllexport) float orbitDist = maxOrbitDist;

__declspec(dllexport) float orbitBumpDist = 0.15f;
__declspec(dllexport) float orbitHalfFovW = 0.15f;
__declspec(dllexport) float orbitHalfFovH = 0.07f;

__declspec(dllexport) float cameraYaw = 0.0f;
__declspec(dllexport) float cameraPitch = 0.5f;

__declspec(dllexport) float frogNewYaw = 0.0f;
__declspec(dllexport) float frogNewPitch = 0.0f;
__declspec(dllexport) int queuedSpin = 0;

__declspec(dllexport) float cameraFocusY = 0.0f;
__declspec(dllexport) float cameraYPrevError = 0.0f;
__declspec(dllexport) float cameraYPidP = 0.1f;
__declspec(dllexport) float cameraYPidD = 0.0f;

// Raycasting
typedef int(__thiscall* raycastFnSig)(char* gameSystem, kcVector4* pPoint, kcVector4* pDirection, float* pDist, int unused, unsigned int mask, unsigned int ignore, _kcSurfaceInfo* pReturnInfo);
__declspec(dllexport) raycastFnSig raycast = reinterpret_cast<raycastFnSig>(0x00416750); // Location in the exe

__declspec(dllexport) int castSteps = 10;

// The expected values of surface info from debugging the original raycast code
_kcSurfaceInfo surfaceInfo = { 0, 0x1F94A278, 0, 0 };

// Helper functions
float clamp(float x, float min, float max) {
    return std::fmin(std::fmax(x, min), max);
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

// Get the mouse input and delta
void __stdcall storeMousePos() {
    // Store the previous positiony
    prevMouseX = mouseX;
    prevMouseY = mouseY;

    // Get the actual mouse coordinate
    POINT mousePos;
    BOOL success = GetCursorPos(&mousePos);

    mouseX = mousePos.x;
    mouseY = mousePos.y;

    // Get deltas if we reset the cursor
    if (captureCursor) {
        SetCursorPos(320, 240);
        mouseDx = mouseX - 320;
        mouseDy = mouseY - 240;
    }
    // Get deltas if we don't reset the cursor
    else {
        mouseDx = mouseX - prevMouseX;
        mouseDy = mouseY - prevMouseY;
    }
}

// Get whether we click the mouse
int __stdcall storeMouseClick() {
    // Check for clicking the mouse
    SHORT mouseDown = GetAsyncKeyState(VK_LBUTTON);
    if (mouseDown < 0) {
        return 0x80;
    }
    return 0;
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
        *gameSystemPtrLoc,
        &origin,
        &normal,
        &dist,
        0,
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

float prevYaw = 0.0f;
float prevPitch = 0.0f;

// Orbiting camera control with mouse
void __stdcall orbitCamera(char* camera, float deltaTime) {
    cameraPtr = camera;

    prevYaw = cameraYaw;
    prevPitch = cameraPitch;

    // Add mouse coordinates and clamp
    cameraYaw += clamp(static_cast<float>(mouseDx) * deltaTime / orbitScaleX, -orbitMaxSpeedX, orbitMaxSpeedX);
    cameraPitch += clamp(static_cast<float>(mouseDy) * deltaTime / orbitScaleY, -orbitMaxSpeedY, orbitMaxSpeedY);

    cameraYaw = wrapToPi(prevYaw * 0.2f + cameraYaw * 0.8f);
    cameraPitch = clamp(prevPitch * 0.2f + cameraPitch * 0.8f, orbitPitchClampOffset, PI - orbitPitchClampOffset);

    frogNewYaw = wrapToPi(-cameraYaw - PI / 2);
    frogNewPitch = clamp(-cameraPitch + PI / 2, -1.55f, 1.55f);

    // Get Frogger's position
    auto frogPos = reinterpret_cast<kcVector4*>(cameraPtr + CAMERA_FROG_POS_OFFSET);
    float frogX = frogPos->x;
    float frogY = frogPos->y;
    float frogZ = frogPos->z;

    // Do PID control on the Y coordinate
    float cameraYError = frogY - cameraFocusY;
    // Teleport for large errors
    if (cameraYError > 5.0f) {
        cameraYError = 0;
        cameraYPrevError = 0;
        cameraFocusY = frogY;
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
void __stdcall freelookCamera(float deltaTime) {
    prevYaw = cameraYaw;
    prevPitch = cameraPitch;

    // Add mouse coordinates and clamp
    cameraYaw += clamp(static_cast<float>(mouseDx) * deltaTime / freelookScaleX, -freelookMaxSpeedX, freelookMaxSpeedX);
    cameraPitch += clamp(static_cast<float>(mouseDy) * deltaTime / freelookScaleY, -freelookMaxSpeedY, freelookMaxSpeedY);

    cameraYaw = wrapToPi(prevYaw * 0.1f + cameraYaw * 0.9f);
    cameraPitch = clamp(prevPitch * 0.1f + cameraPitch * 0.9f, orbitPitchClampOffset, PI - orbitPitchClampOffset);

    // Convert to freelook coordinates and set the camera's pitch and yaw
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
void __stdcall applyQueuedSpin() {
    if (queuedSpin) {
        // Set the PID parameters for Frogger's yaw rotation, so he turns away from the camera
        auto frogYawPid = reinterpret_cast<_kcPid*>(frogPtr + FROG_TURN_PID_OFFSET);
        frogYawPid->x = frogNewYaw;
        frogYawPid->eq = frogNewYaw;
        frogYawPid->v = 0;
        frogYawPid->i = 0;

        // Set the analogue input to forward, forcing Frogger to move forward a slight amount and rotate towards the camera
        *analogueYInputPtr = 1.0f;

        --queuedSpin;
    }
}
