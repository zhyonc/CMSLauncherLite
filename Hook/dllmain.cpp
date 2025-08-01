// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Hook.h"

static HANDLE ghThread = nullptr;

DWORD WINAPI MainProc(LPVOID lpParameter) {
	Hook::Install();
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		ghThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainProc, nullptr, 0, nullptr);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		// No specific handling
		break;
	case DLL_PROCESS_DETACH:
		if (ghThread) {
			WaitForSingleObject(ghThread, INFINITE);
			CloseHandle(ghThread);
			ghThread = nullptr;
		}
		Hook::Uninstall();
		break;
	}
	return TRUE;
}

