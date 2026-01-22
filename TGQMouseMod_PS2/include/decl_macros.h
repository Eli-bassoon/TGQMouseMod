#pragma once

// How to export variables and functions
#ifdef PS2
// PS2
#define EXPORT_VAR
#define EXPORT_FUNC(rettype) extern "C" rettype

#else
// PC
#define EXPORT_VAR __declspec(dllexport)
#define EXPORT_FUNC(rettype) rettype __declspec(dllexport) __stdcall

#endif