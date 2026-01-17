#pragma once

#include <Windows.h>
#include <cstdint>
#include <cassert>

// A struct that encodes an unconditional relative jump in its memory layout
#pragma pack(push, 1)
struct x86Jump
{
    const uint8_t cmd = 0xE9; // Relative jump opcode
    uintptr_t addr;
};
#pragma pack(pop)

// Injects code at a point and returns to after that point when the injected code is finished executing
inline void Inject(uintptr_t injectAddr, uintptr_t destAddr, int overwriteLen) {
    static_assert(sizeof(x86Jump) == 5, "Jump struct not expected size.");
    assert(overwriteLen >= 5);

    // Make injection site readable
    DWORD curProtection;
    VirtualProtect((LPVOID)injectAddr, overwriteLen, PAGE_EXECUTE_READWRITE, &curProtection);

    // Fill with NOPs now
    memset((LPVOID)injectAddr, 0x90, overwriteLen);

    // Patch bytes at injection site to redirect to new code
    x86Jump cmd;
    cmd.addr = destAddr - injectAddr - sizeof(cmd);
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)injectAddr, &cmd, sizeof(cmd), nullptr);

    // Reset protection
    DWORD newProtection;
    VirtualProtect((LPVOID)injectAddr, overwriteLen, curProtection, &newProtection);
}

// Helper macro to write inline assembly inside a macro
#define ASM(...) __VA_ARGS__

// Macro to generate the injection code
#define DEFINE_INJECT_WRAPPER(name, jumpAddr, numPatchBytes, MID_ASM) \
constexpr uintptr_t name##__jumpFromAddr = jumpAddr; \
constexpr unsigned int name##__numPatchBytes = numPatchBytes; \
__declspec(naked) void name##__injectable() { \
    static constexpr DWORD afterInjectAddr = jumpAddr + numPatchBytes; \
    MID_ASM \
    __asm jmp [afterInjectAddr] \
}

// Macro that actually injects, MUST have a corresponding define inject wrapper already
#define USE_INJECT_WRAPPER(name) Inject(name##__jumpFromAddr, reinterpret_cast<uintptr_t>(name##__injectable), name##__numPatchBytes)