#include <Windows.h>
#include "injection_functions.h"

#ifdef PROXIED
#include "dsound.h"
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
#ifdef PROXIED
        // Initialize the DLL proxy
        dllforward::setup();
#endif

#ifdef AUTOINJECT
        SetupHooks();
#endif
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

