#include <utility>
#include <Windows.h>
#include "mouse_utils.h"

__declspec(dllexport) int prevMouseX = 0;
__declspec(dllexport) int prevMouseY = 0;
__declspec(dllexport) int mouseX = 0;
__declspec(dllexport) int mouseY = 0;

__declspec(dllexport) int mouseDx = 0;
__declspec(dllexport) int mouseDy = 0;

// How much to scale the mouse, to be about as sensitive as the PS2 emulated mouse
__declspec(dllexport) float mouseScaling = 0.4f;

__declspec(dllexport) bool captureCursor = true;

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
        return 0x80; // This signals the attack button was pressed
    }
    return 0;
}

std::pair<float, float> getCameraDeltas() {
    return { mouseDx * mouseScaling, mouseDy * mouseScaling };
}
