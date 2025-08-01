// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <winsock2.h>   
#include <dinput.h>
#include <vector>
#include <string>

#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

#pragma region lib

#ifndef IS_DEBUG_MODE
#define IS_DEBUG_MODE true
#endif

#include "MapleLib.h"
#pragma comment(lib, "MapleLib.lib")

#pragma endregion

#endif
