#pragma once
#include <windows.h>

BOOL SetHook(BOOL bInstall, PVOID* ppPointer, PVOID pDetour);

// Usage: SADDR(&_FuncName,uAddress);
template <typename T>
inline void SADDR(T** ppPointer, ULONG_PTR uAddress) {
	*ppPointer = reinterpret_cast<T*>(uAddress);
}

// Usage: SHOOK(ture,&_FuncName,FuncName_Hook);
template <typename T>
inline BOOL SHOOK(BOOL bInstall, T** ppPointer, PVOID pDetour) {
	 return SetHook(bInstall, reinterpret_cast<void**>(ppPointer), pDetour);
}