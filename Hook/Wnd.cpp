
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "Wnd.h"
#include "Tool.h"

#include "Resources/AOBList.h"
#include "Share/Funcs.h"

namespace {

	static void** CInputSystemInstancePtr = nullptr; // TSingleton<CInputSystem>::ms_pInstance
	static IDirectInputDevice8A* pMouseDevice = nullptr;

	static int(__thiscall* _CWndMan__TranslateMessage)(void* ecx, unsigned int* message, unsigned int* wParam, int* lParam, int* plResult) = nullptr;
	int __fastcall CWndMan__TranslateMessage_Hook(void* ecx, void* edx, unsigned int* message, unsigned int* wParam, int* lParam, int* plResult) {
		if (CInputSystemInstancePtr && !pMouseDevice) {
			void* CInputSystemInstance = *CInputSystemInstancePtr;
			if (CInputSystemInstance) {
				pMouseDevice = *(IDirectInputDevice8A**)((ULONG_PTR)CInputSystemInstance + 0xC);
			}
		}
		if (pMouseDevice && *message == WM_MOUSEMOVE) {
			pMouseDevice->Acquire();
		}
		return _CWndMan__TranslateMessage(ecx, message, wParam, lParam, plResult);
	}
}

namespace Wnd {

	bool SkipStartUpLogo(Rosemary& r)
	{
		ULONG_PTR uAddr = r.Scan(AOB_Scan_CLogo_Init_Addr);
		if (uAddr == 0) {
			DEBUG(L"Failed to find CLogo::Init");
			return false;
		}
		return r.FillBytes(uAddr, 0x90, 20);
	}

	bool FixWindowMode(Rosemary& r) {
		// CMS15 CWvsApp::InitializeGr2D
		// GMS95 CAnimationDisplayer::SetLocalFadeLayer
		ULONG_PTR uAddr = r.Scan(AOB_Scan_Window_Mode_Addr);
		if (uAddr == 0) {
			DEBUG(L"Failed to find CWvsApp::InitializeGr2D");
			return false;
		}
		ULONG_PTR Window_Mode = uAddr + 0x3;
		return r.WriteByte(Window_Mode, 0x00);
	}

	bool FixCursorFocus(Rosemary& r) {
		// SetCooperativeLevel from (0x5) DISCL_EXCLUSIVE | DISCL_FOREGROUND 
		// to (0x6) DISCL_NONEXCLUSIVE | DISCL_FOREGROUND
		ULONG_PTR uSetCooperativeLevelAddr = r.Scan(AOB_Scan_SetCooperativeLevel_Addr);
		if (uSetCooperativeLevelAddr == 0) {
			DEBUG(L"Failed to find SetCooperativeLevel");
			return false;
		}
		ULONG_PTR uTranslateMessageAddr = r.Scan(AOB_Scan_CWndMan__TranslateMessage_Addr);
		if (uTranslateMessageAddr == 0) {
			DEBUG(L"Failed to find CWndMan::TranslateMessage");
			return false;
		}
		CInputSystemInstancePtr = (void**)GetDWORDAddress(uTranslateMessageAddr + 0x16);
		r.WriteByte(uSetCooperativeLevelAddr + 0x5, 0x06);
		SADDR(&_CWndMan__TranslateMessage, uTranslateMessageAddr);
		SHOOK(true, &_CWndMan__TranslateMessage, CWndMan__TranslateMessage_Hook);
		return true;
	}

}