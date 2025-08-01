#include "pch.h"
#include "Hook.h"
#include "AntiCheat.h"
#include "ResMan.h"
#include "Wnd.h"

#include "Resources/AOBList.h"
#include "Resources/Config.h"

namespace {
	static Rosemary gMapleR;
	static bool gHooked = false;
	static bool bGethostbynameLoaded = false;
	static bool bCreateMutexALoaded = false;

	// Make sure the executable unpacks itself.
	bool InitMapleSectionList() {
		if (gMapleR.IsSectionInitialized()) {
			DEBUG(L"Maple section list was already initialized");
			return true;
		}
		gMapleR.InitSectionList(L"MapleStory.exe");
		if (!gMapleR.IsSectionInitialized()) {
			DEBUG(L"Failed to init maple section list");
			return false;
		}
		DEBUG(L"Maple section list init ok");
		return true;
	}

	static auto _GetACP = decltype(&GetACP)(GetProcAddress(GetModuleHandleW(L"KERNEL32"), "GetACP"));
	UINT WINAPI GetACP_Hook() {
		// Fake Locale Check
		return 936;
	}

	static auto _gethostbyname = decltype(&gethostbyname)(GetProcAddress(LoadLibraryW(L"Ws2_32"), "gethostbyname"));
	hostent* WINAPI gethostbyname_Hook(const char* name) {
		// Redirect connect addr
		if (!bGethostbynameLoaded) {
			bGethostbynameLoaded = true;
			// Versions above CMS48 can't use AOBScan during startup
			// Please use gMapleR.WriteCode(uAddress, wCode) as a replacement for AOBScan  
			// TODO Bypass AntiCheat and set WindowMode here
		}
		return _gethostbyname(Config::ServerAddr.c_str());
	}

	// CreateMutexA is the first Windows library call after the executable unpacks itself. 
	// It is recommended to have all Maple hooking and memory editing inside or called from the CreateMutexA function.
	static auto _CreateMutexA = decltype(&CreateMutexA)(GetProcAddress(GetModuleHandleW(L"KERNEL32"), "CreateMutexA"));
	HANDLE WINAPI CreateMutexA_Hook(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName) {
		if (!bCreateMutexALoaded && InitMapleSectionList()) {
			bCreateMutexALoaded = true;
			// AntiCheat
			AntiCheat::RemoveSecurityChecks(gMapleR);
			if (!AntiCheat::RemoveNewPatcherCheck(gMapleR)) {
				DEBUG(L"Unable to remove security thread");
			}
			if (!AntiCheat::RemoveDisconnectException(gMapleR)) {
				DEBUG(L"Unable to remove termination error");
			}
			if (!AntiCheat::RemoveSecurityClient(gMapleR)) {
				DEBUG(L"Unable to completely remove the security client");
			}
			if (!AntiCheat::RemoveSecurityClientInitFailedException(gMapleR)) {
				DEBUG(L"Unable to remove AVCSecurityInitFailed exception");
			}
			// Game window opened
			if (!Wnd::SkipStartUpLogo(gMapleR)) {
				DEBUG(L"Failed to skip logo");
			}
			if (!Wnd::FixWindowMode(gMapleR)) {
				DEBUG(L"Unable to fix window mode");
			}
			if (!Wnd::FixCursorFocus(gMapleR)) {
				DEBUG(L"Unable to fix cursor focus");
			}
			// Load game data
			if (!ResMan::Mount(gMapleR)) {
				DEBUG(L"Unable to mount res man");
			}
			//RShield::Init();
		}
		if (lpName && strstr(lpName, "WvsClientMtx")) {
			// MultiClient: faking HANDLE is 0xBADF00D(BadFood)
			return (HANDLE)0xBADF00D;
		}
		return _CreateMutexA(lpMutexAttributes, bInitialOwner, lpName);
	}

	static auto _CreateWindowExA = decltype(&CreateWindowExA)(GetProcAddress(GetModuleHandleW(L"USER32"), "CreateWindowExA"));
	HWND WINAPI CreateWindowExA_Hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
		if (lpClassName && strstr(lpClassName, "MapleStoryClass")) {
			// Customize game window title 
			lpWindowName = Config::WindowTitle.c_str();
			// Show minimize button
			dwStyle |=  WS_MINIMIZEBOX | WS_SYSMENU;
			// Place the game window in the center of the screen
			RECT screenRect;
			GetWindowRect(GetDesktopWindow(), &screenRect);
			int centerX = screenRect.right / 2 - nWidth / 2;
			int centerY = screenRect.bottom / 2 - nHeight / 2;
			return _CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, centerX, centerY, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		}
		return _CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}

}

namespace Hook {

	void Install() {
		bool gHooked = SHOOK(true, &_GetACP, GetACP_Hook) &&
			SHOOK(true, &_gethostbyname, gethostbyname_Hook) &&
			SHOOK(true, &_CreateMutexA, CreateMutexA_Hook) &&
			SHOOK(true, &_CreateWindowExA, CreateWindowExA_Hook);
		if (!gHooked) {
			DEBUG(L"Failed to install hook");
			return;
		}
		DEBUG(L"Hook install ok");
	}

	void Uninstall() {
		if (!gHooked) {
			return;
		}
		bool ok = SHOOK(false, &_GetACP, GetACP_Hook) &&
			SHOOK(false, &_gethostbyname, gethostbyname_Hook) &&
			SHOOK(false, &_CreateMutexA, CreateMutexA_Hook) &&
			SHOOK(false, &_CreateWindowExA, CreateWindowExA_Hook);
		if (!ok) {
			DEBUG(L"Failed to uninstall hook");
			return;
		}
		DEBUG(L"Hook uninstall ok");
	}
}
