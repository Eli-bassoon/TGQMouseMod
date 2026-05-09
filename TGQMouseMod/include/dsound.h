// DLLForward by itisluiz v1.3
#pragma once
#include <cstdint>
#include <Windows.h>

namespace dllforward
{
	namespace internal
	{
		struct Export
		{
			void(*method)();
			const char* name;
			const uint16_t ordinal;
			const uint32_t rva;
		};

#pragma optimize("", off)

		static volatile uint16_t volatileWord;
		static __declspec(noinline) void __CALL_DUMMY() { volatileWord = 0; }

		// Proxy header generated for dsound.dll (32 bit)
		static_assert(sizeof(void*) == 4, "The proxied DLL must match the architecture of the proxy DLL");

		// #6: DirectSoundCaptureCreate (DirectSoundCaptureCreate)
		void __EXPORT_DUMMY6()
		{
#pragma comment(linker, "/EXPORT:DirectSoundCaptureCreate=" __FUNCDNAME__ ",@6")
			__CALL_DUMMY();
			volatileWord = 6;
		}

		// #12: DirectSoundCaptureCreate8 (DirectSoundCaptureCreate8)
		void __EXPORT_DUMMY12()
		{
#pragma comment(linker, "/EXPORT:DirectSoundCaptureCreate8=" __FUNCDNAME__ ",@12")
			__CALL_DUMMY();
			volatileWord = 12;
		}

		// #7: DirectSoundCaptureEnumerateA (DirectSoundCaptureEnumerateA)
		void __EXPORT_DUMMY7()
		{
#pragma comment(linker, "/EXPORT:DirectSoundCaptureEnumerateA=" __FUNCDNAME__ ",@7")
			__CALL_DUMMY();
			volatileWord = 7;
		}

		// #8: DirectSoundCaptureEnumerateW (DirectSoundCaptureEnumerateW)
		void __EXPORT_DUMMY8()
		{
#pragma comment(linker, "/EXPORT:DirectSoundCaptureEnumerateW=" __FUNCDNAME__ ",@8")
			__CALL_DUMMY();
			volatileWord = 8;
		}

		// #1: DirectSoundCreate (DirectSoundCreate)
		void __EXPORT_DUMMY1()
		{
#pragma comment(linker, "/EXPORT:DirectSoundCreate=" __FUNCDNAME__ ",@1")
			__CALL_DUMMY();
			volatileWord = 1;
		}

		// #11: DirectSoundCreate8 (DirectSoundCreate8)
		void __EXPORT_DUMMY11()
		{
#pragma comment(linker, "/EXPORT:DirectSoundCreate8=" __FUNCDNAME__ ",@11")
			__CALL_DUMMY();
			volatileWord = 11;
		}

		// #2: DirectSoundEnumerateA (DirectSoundEnumerateA)
		void __EXPORT_DUMMY2()
		{
#pragma comment(linker, "/EXPORT:DirectSoundEnumerateA=" __FUNCDNAME__ ",@2")
			__CALL_DUMMY();
			volatileWord = 2;
		}

		// #3: DirectSoundEnumerateW (DirectSoundEnumerateW)
		void __EXPORT_DUMMY3()
		{
#pragma comment(linker, "/EXPORT:DirectSoundEnumerateW=" __FUNCDNAME__ ",@3")
			__CALL_DUMMY();
			volatileWord = 3;
		}

		// #10: DirectSoundFullDuplexCreate (DirectSoundFullDuplexCreate)
		void __EXPORT_DUMMY10()
		{
#pragma comment(linker, "/EXPORT:DirectSoundFullDuplexCreate=" __FUNCDNAME__ ",@10")
			__CALL_DUMMY();
			volatileWord = 10;
		}

		// #4: DllCanUnloadNow (DllCanUnloadNow)
		void __EXPORT_DUMMY4()
		{
#pragma comment(linker, "/EXPORT:DllCanUnloadNow=" __FUNCDNAME__ ",@4")
			__CALL_DUMMY();
			volatileWord = 4;
		}

		// #5: DllGetClassObject (DllGetClassObject)
		void __EXPORT_DUMMY5()
		{
#pragma comment(linker, "/EXPORT:DllGetClassObject=" __FUNCDNAME__ ",@5")
			__CALL_DUMMY();
			volatileWord = 5;
		}

		// #9: GetDeviceID (GetDeviceID)
		void __EXPORT_DUMMY9()
		{
#pragma comment(linker, "/EXPORT:GetDeviceID=" __FUNCDNAME__ ",@9")
			__CALL_DUMMY();
			volatileWord = 9;
		}

#pragma optimize("", on)

		constexpr wchar_t originalProxiedDll[]{ L"C:\\Windows\\SysWOW64\\dsound.dll" };
		constexpr Export exports[]{ { __EXPORT_DUMMY6, "DirectSoundCaptureCreate", 6, 0x2cc60 }, { __EXPORT_DUMMY12, "DirectSoundCaptureCreate8", 12, 0x2caf0 }, { __EXPORT_DUMMY7, "DirectSoundCaptureEnumerateA", 7, 0x2cdc0 }, { __EXPORT_DUMMY8, "DirectSoundCaptureEnumerateW", 8, 0x2ce30 }, { __EXPORT_DUMMY1, "DirectSoundCreate", 1, 0x2cea0 }, { __EXPORT_DUMMY11, "DirectSoundCreate8", 11, 0x1e800 }, { __EXPORT_DUMMY2, "DirectSoundEnumerateA", 2, 0x2d000 }, { __EXPORT_DUMMY3, "DirectSoundEnumerateW", 3, 0x2d070 }, { __EXPORT_DUMMY10, "DirectSoundFullDuplexCreate", 10, 0x2d0e0 }, { __EXPORT_DUMMY4, "DllCanUnloadNow", 4, 0x3b420 }, { __EXPORT_DUMMY5, "DllGetClassObject", 5, 0x14f20 }, { __EXPORT_DUMMY9, "GetDeviceID", 9, 0x2d520 } };
	}

	static HMODULE setup()
	{
#ifdef DLLFORWARD_ORIGINALDLLPATH_W
		HMODULE hProxiedDLL{ LoadLibraryW(DLLFORWARD_ORIGINALDLLPATH_W) };
#elif defined(DLLFORWARD_ORIGINALDLLPATH)
		HMODULE hProxiedDLL{ LoadLibraryA(DLLFORWARD_ORIGINALDLLPATH) };
#else
		HMODULE hProxiedDLL{ LoadLibraryW(internal::originalProxiedDll) };
#endif

		if (!hProxiedDLL)
			return NULL;

		for (const internal::Export& exportEntry : internal::exports)
		{
#ifdef DLLFORWARD_RESOLVEPROC_RVA
			uintptr_t pProxiedMethod{ reinterpret_cast<uintptr_t>(hProxiedDLL) + exportEntry.rva };
#elif defined(DLLFORWARD_RESOLVEPROC_ORDINAL)
			uintptr_t pProxiedMethod{ reinterpret_cast<uintptr_t>(GetProcAddress(hProxiedDLL, MAKEINTRESOURCEA(exportEntry.ordinal))) };
#else
			uintptr_t pProxiedMethod{ reinterpret_cast<uintptr_t>(GetProcAddress(hProxiedDLL, exportEntry.name)) };
#endif

			uintptr_t pProxyMethod{ reinterpret_cast<uintptr_t>(exportEntry.method) };

#ifdef _DEBUG
			if (*reinterpret_cast<uint8_t*>(pProxyMethod) == 0xE9)
			{
				int32_t offset{ *reinterpret_cast<int32_t*>(pProxyMethod + 1) };
				pProxyMethod += offset + 5;
			}
#endif

#ifdef _WIN64
			uint8_t opcodes[14]{ 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };
			*reinterpret_cast<uintptr_t*>(opcodes + 6) = pProxiedMethod;
#else
			uint8_t opcodes[5]{ 0xE9 };
			*reinterpret_cast<uintptr_t*>(opcodes + 1) = pProxiedMethod - pProxyMethod - sizeof(opcodes);
#endif

			DWORD originalProtect, newProtect;
			uint8_t* pProxyMethodBytes{ reinterpret_cast<uint8_t*>(pProxyMethod) };
			VirtualProtect(pProxyMethodBytes, sizeof(opcodes), PAGE_EXECUTE_READWRITE, &originalProtect);
			memcpy_s(pProxyMethodBytes, sizeof(opcodes), opcodes, sizeof(opcodes));
			VirtualProtect(pProxyMethodBytes, sizeof(opcodes), originalProtect, &newProtect);
		}

		return hProxiedDLL;
	}
}
