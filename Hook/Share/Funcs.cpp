#include "Funcs.h"

#include <detours.h>
#pragma comment(lib, "detours.lib")

BOOL SetHook(BOOL bInstall, PVOID* ppPointer, PVOID pDetour) {
	if (DetourTransactionBegin() != NO_ERROR) {
		return FALSE;
	}

	auto tid = GetCurrentThread();

	if (DetourUpdateThread(tid) != NO_ERROR) {
		DetourTransactionAbort();
		return FALSE;
	}

	auto func = bInstall ? DetourAttach : DetourDetach;

	if (func(ppPointer, pDetour) != NO_ERROR) {
		DetourTransactionAbort();
		return FALSE;
	}

	if (DetourTransactionCommit() != NO_ERROR) {
		DetourTransactionAbort();
		return FALSE;
	}

	return TRUE;
}