#include    <stdio.h>
#include    <string>
#include    <windows.h>
#include    <iostream>

// From https://everthessel.nl/blog/dll-injection-function-hooking/
int main(int argc, char* argv[])
{
	// The user can pass in a path to the executable. By default, it assumes the exe is in the same directory as the injector.
	const char* exe;
	if (argc == 1) {
		exe = "GreatQuest.exe";
	}
	else if (argc == 2) {
		std::string arg1 = std::string(argv[1]);
		// See if the user is asking for help
		if (arg1 == "help" || arg1 == "h" || arg1 == "--help" || arg1 == "-h") {
			std::cout << "Injector - Inject TGQMouseMod into the Great Quest executable" << std::endl;
			std::cout << "Usage:" << std::endl;
			std::cout << "\tInjector.exe [path]" << std::endl;
			std::cout << "With no arguments passed, assume that the injector is placed in the same folder as the executable." << std::endl;
			return 0;
		}
		// Otherwise, set the exe path
		else {
			exe = argv[1];
		}
	}
	else {
		std::cerr << "Please enter only one argument for the executable path.";
		return 1;
	}
	// Path to our application
	const char* dll = "TGQMouseMod.dll";

	// Arguments for our application, including name of the executable itself
	char args[20] = "GreatQuest.exe";

	// Address to the main function of our application
	LPVOID entrypoint = (LPVOID)0x0048dba2;

	// Holds properties of the started process
	STARTUPINFOA StartupInfo = { 0 };
	// For backwards/forwards compatibility Windows requires us to fill in the size of the StartupInfo instance
	StartupInfo.cb = sizeof(StartupInfo);

	// Also holds properties of the started process
	PROCESS_INFORMATION ProcessInformation;

	// Variables for tracking the memory protection of the process
	DWORD originalProtection;
	DWORD unusedProtection;

	// Holds the original bytes that we will replace with an infinite loop
	char originalBytes[2];
	// Infinite loop instruction
	static const char infiniteLoop[2] = { '\xEB', '\xFE' };
	// Generic size variable, stores the number of bytes written/read using ReadProcessMemory/WriteProcessMemory
	SIZE_T size;
	// Handle of the loaded DLL
	HANDLE LoadDLL;
	// Handle of the Kernel32 module
	HMODULE Kernel32;
	// Full path to the DLL
	char dllPath[_MAX_PATH];
	// Pointer to the DLL path in the process
	void* remoteDllPath;

	// Spawn the application and immediately suspend it
	if (CreateProcessA(
		exe,
		args,
		NULL,
		NULL,
		FALSE,
		CREATE_SUSPENDED,
		NULL,
		NULL,
		&StartupInfo,
		&ProcessInformation
	)
		) {
		HANDLE hProcess(ProcessInformation.hProcess);

		try {
			// Entry point of the application
			LPVOID entry = entrypoint;

			// Make the memory of the process writeable
			VirtualProtectEx(hProcess, entry, 2, PAGE_EXECUTE_READWRITE, &originalProtection);

			// Save the original bytes at the entry point and replace them with a JMP $-2 / infinite loop
			ReadProcessMemory(hProcess, entry, originalBytes, 2, &size);
			WriteProcessMemory(hProcess, entry, infiniteLoop, 2, &size);
			VirtualProtectEx(hProcess, entry, 2, originalProtection, &unusedProtection);

			// Resume the process
			ResumeThread(ProcessInformation.hThread);

			// Wait until the process has reached the infinite loop
			for (;;) {
				// Check if the Instruction Pointer points to our infinite loop
				CONTEXT context;
				context.ContextFlags = CONTEXT_CONTROL;
				GetThreadContext(ProcessInformation.hThread, &context);

				// If yes, exit the loop
				if (context.Eip == (DWORD)entry) {
					break;
				}

				// Else, sleep for a moment and then try again
				Sleep(100);
			}

			// Get the full path to the DLL
			GetFullPathNameA(dll, _MAX_PATH, dllPath, NULL);

			// Allocate memory for the DLL path in the application process
			remoteDllPath = VirtualAllocEx(hProcess, NULL, sizeof(dllPath), MEM_COMMIT, PAGE_READWRITE);
			if (remoteDllPath == nullptr) {
				throw;
			}

			// Write the DLL path to the allocated memory
			WriteProcessMemory(hProcess, remoteDllPath, (void*)dllPath, sizeof(dllPath), NULL);

			// Get a handle to the Kernel32 module
			Kernel32 = GetModuleHandleA("Kernel32");
			if (Kernel32 == nullptr) {
				throw;
			}

			// Call LoadLibraryA from the application process and pass the DLL path as a parameter
			LoadDLL = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(Kernel32, "LoadLibraryA"), remoteDllPath, 0, NULL);
			if (LoadDLL == nullptr) {
				throw;
			}

			// Wait for the DLL to load and return.
			WaitForSingleObject(LoadDLL, INFINITE);
			CloseHandle(LoadDLL);

			// Free the memory used for storing the DLL path in the application process
			VirtualFreeEx(hProcess, remoteDllPath, 0, MEM_RELEASE);

			// Suspend the process and make its memory writeable again
			SuspendThread(ProcessInformation.hThread);
			VirtualProtectEx(hProcess, entry, 2, PAGE_EXECUTE_READWRITE, &originalProtection);

			// Restore the original bytes
			WriteProcessMemory(hProcess, entry, originalBytes, 2, &size);

			// Restore the original memory protection
			VirtualProtectEx(hProcess, entry, 2, originalProtection, &unusedProtection);

			// Resume the patched application
			ResumeThread(ProcessInformation.hThread);

			std::cout << "Successfully injected DLL\n";
		}
		catch (...) {
			// An error occurred, kill the spawned process
			TerminateProcess(hProcess, -1);
			std::cerr << "Error while injecting DLL\n";
			return 1;
		}
	}
	else {
		std::cout << "Application not found\n";
		return 1;
	}

	return 0;
}