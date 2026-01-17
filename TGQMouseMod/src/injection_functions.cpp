#include "mod_functions.h"
#include "hooking.h"

// Get the mouse input and clicks
DEFINE_INJECT_WRAPPER(
	InjectHookMouseInput,
	0x0040a03d,
	7,
	ASM(

__asm call storeMousePos

__asm call storeMouseClick
__asm mov ebx, eax

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

__asm mov eax, [esi + 0x268]
__asm mov[esp + 0x1C], eax
__asm mov eax, [cameraFocusY]
__asm mov[esp + 0x20], eax
__asm mov eax, [esi + 0x270]

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
__asm mov ebx,[esi+0xC8]

	)
)

// Transition instantly from orbit to freelook
constexpr uintptr_t InjectFreelookTransitionFld = 0x004B7764;
DEFINE_INJECT_WRAPPER(
	InjectFreelookTransition,
	0x00446D4D,
	6,
	ASM(

__asm mov eax, [esi + 0x17c]
__asm mov[esi + 0x178], eax

__asm mov eax, [esi + 0x19c]
__asm mov [esi + 0x198], eax

__asm mov eax, [esi + 0x1bc]
__asm mov [esi + 0x1b8], eax

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

__asm call testApplyQueuedSpin

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
