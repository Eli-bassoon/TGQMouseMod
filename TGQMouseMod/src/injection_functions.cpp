#include "symbols_pc.h"
#include "hooking.h"
#include "mouse_utils.h"

// When modifying one of these hooks, also remember to modify the corresponding hook in the cheat table

// Get the mouse input and clicks
DEFINE_INJECT_WRAPPER(
	InjectHookMouseInput,
	0x0040a03d,
	7,
	ASM(

__asm call storeMousePos

__asm call storeMouseClick
__asm mov ebx, eax // The hooked function expects the result in ebx

// Original code
__asm xor esi, esi
__asm call edi
__asm test ah, -80

	)
)

// Look directly at Frogger
DEFINE_INJECT_WRAPPER(
	InjectLookAtFrogger,
	0x004471B7,
	26,
	ASM(

// The camera PID tries to look at frogger, but the PID smoothing normally causes a lag
// esi is cameraPtr, and we offset to Frogger's actual position
// Here we move the actual x and z position of frogger, along with our own PID smoothed y position
// to the local variables on the stack corresponding to the focus point
// Later, the hooked function computes a rotation matrix to aim the camera directly at this focus point
__asm mov eax, [esi + CAMERA_FROG_POS_OFFSET]
__asm mov [esp + 0x1C], eax
__asm mov eax, [cameraFocusY]
__asm mov [esp + 0x20], eax
__asm mov eax, [esi + CAMERA_FROG_POS_OFFSET + 0x8]

	)
)

// Move the orbit camera
// This one is special and jumps to a different spot
constexpr uintptr_t InjectOrbitCameraActualJump = 0x00446345;
DEFINE_INJECT_WRAPPER(
	InjectOrbitCamera,
	0x00446382,
	6,
	ASM(

// esi is cameraPtr
__asm push esi
__asm call orbitCamera
__asm jmp [InjectOrbitCameraActualJump]

	)
)

// Get the address of Frogger's class so we can directly modify him
DEFINE_INJECT_WRAPPER(
	InjectGetFrogPointer,
	0x00470113,
	7,
	ASM(

__asm mov [frogPtr],esi

// Original code
__asm push edi
__asm mov ebx, [esi + 0xC8]

	)
)

// Transition instantly from orbit to freelook
constexpr uintptr_t InjectFreelookTransitionFld = 0x004B7764;
DEFINE_INJECT_WRAPPER(
	InjectFreelookTransition,
	0x00446D4D,
	6,
	ASM(

// esi is cameraPtr
// 0x178 is the offset to the start of the position PID structs
// The structs are aligned so +0 is the current position and +4 is the goal position
// Each struct's size is 0x20 bytes
// So these lines move the goal position directly to the current position
// This bypasses the PID and sets the current position directly to the goal position
__asm mov eax, [esi + CAMERA_PID_OFFSET + 0x4]
__asm mov [esi + CAMERA_PID_OFFSET], eax

__asm mov eax, [esi + CAMERA_PID_OFFSET + 0x20 + 0x4]
__asm mov [esi + CAMERA_PID_OFFSET + 0x20], eax

__asm mov eax, [esi + CAMERA_PID_OFFSET + 0x40 + 0x4]
__asm mov [esi + CAMERA_PID_OFFSET + 0x40], eax

// Original code
//__asm fld dword ptr [InjectFreelookTransitionFld]
__asm mov eax,[InjectFreelookTransitionFld]
__asm fld dword ptr [eax]

	)
)

// Aim the freelook camera with the mouse
DEFINE_INJECT_WRAPPER(
	InjectFreelookCamera,
	0x00446ED9,
	60,
	ASM(

__asm call freelookCamera

	)
)

// Fire goobers where Frogger is looking in orbit mode
DEFINE_INJECT_WRAPPER(
	InjectAimGoobersOrbit,
	0x0046D9E3,
	6,
	ASM(

// Queue moving Frogger forward next frame
__asm mov [queuedSpin],1

// Get the yaw
__asm mov edx,[frogNewYaw]

	)
)

// Apply queued spin after firing a goober, so Frogger actually stays rotated at that location
DEFINE_INJECT_WRAPPER(
	InjectApplyQueuedSpin,
	0x00471336,
	7,
	ASM(

__asm call applyQueuedSpin

// Original code
__asm push -1
__asm push 0x004ab670

	)
)

void SetupHooks() {
	USE_INJECT_WRAPPER(InjectHookMouseInput);
	USE_INJECT_WRAPPER(InjectLookAtFrogger);
	USE_INJECT_WRAPPER(InjectOrbitCamera);
	USE_INJECT_WRAPPER(InjectGetFrogPointer);
	USE_INJECT_WRAPPER(InjectFreelookTransition);
	USE_INJECT_WRAPPER(InjectFreelookCamera);
	USE_INJECT_WRAPPER(InjectAimGoobersOrbit);
	USE_INJECT_WRAPPER(InjectApplyQueuedSpin);
}
