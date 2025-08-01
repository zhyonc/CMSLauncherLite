#include "pch.h"
#include "AntiCheat.h"

#include "Resources/AOBList.h"

namespace AntiCheat {

	void RemoveSecurityChecks(Rosemary& r)
	{
#pragma region WinMain
		// Description: Strip out the activation of MapleStoryAcGuardian events
		// CMS15(00590C29) v6 = strncmp(lpCmdLine, "MapleStoryAcGuardian", strlen("MapleStoryAcGuardian"))
		// CMS15(00590C3A) if(!v6) { return 0; }
		// CMS15(00590CA5) MutexA = CreateMutexA(0, 1, "MapleStoryMutex");
		ULONG_PTR uMapleStoryAcGuardianActivationAddr = r.Scan(AOB_Scan_MapleStoryAcGuardian_Activation_Addr);
		if (uMapleStoryAcGuardianActivationAddr > 0) {
			r.WriteByte(uMapleStoryAcGuardianActivationAddr, 0xEB);
		}
#pragma endregion
#pragma region CWvsApp::SetUp
#pragma region CWvsApp::InitializeResMan
		// Description: Strip out CRC integrity checks
		// CMS15(00592F4A) if(Crc32!=variant.lVal){ _com_issue_error(-2147467259); }
		// CMS15(00592F56) if ( !dword_610238[dword_610258 & 7] )
		ULONG_PTR uIntegrityCheckAddr = r.Scan(AOB_Scan_CWvsApp__InitializeResMan_Integrity_Check_Addr);
		if (uIntegrityCheckAddr > 0) {
			r.WriteByte(uIntegrityCheckAddr, 0xEB);
		}
#pragma endregion
		// Description: Strip out the creation of MapleStoryAcGuardian events.
		// Description: Strip out the CSecurityThread::Update thread start function.
		// CMS15(00592179) set_stage(v5, 0);
		// CMS15(00592182) CWvsApp::InitAcGuardian((int*)this);
		// CMS15(00592189) CSecurityThread::Start()
		ULONG_PTR uCWvsAppInitAcGuardianAddr = r.Scan(AOB_Scan_CWvsApp__InitAcGuardian_Addr);
		if (uCWvsAppInitAcGuardianAddr > 0) {
			r.WriteCode(uCWvsAppInitAcGuardianAddr, AOB_Code_Ret1);
		}
		ULONG_PTR uCSecurityThreadStartAddr = r.Scan(AOB_Scan_CSecurityThread__Start_Addr);
		if (uCSecurityThreadStartAddr > 0) {
			r.WriteCode(uCSecurityThreadStartAddr, AOB_Code_Ret1);
		}
#pragma endregion
#pragma region CWvsApp::Run
		// Description Strip out the CSecurityThread ack timeout checks from CWvsApp::CallUpdate
		// CMS15(005921AA) CWvsApp::Run
		// CMS15(005922C9) CWvsApp::CallUpdate(v5)
		// CMS15(00593B49) if ( !(++dword_612B8C % 0x64u) )( CWvsApp::UpdateTime(); ) 
		ULONG_PTR uCWvsAppUpdateTimeCall = r.Scan(AOB_Scan_CWvsApp__UpdateTime_Call);
		if (uCWvsAppUpdateTimeCall > 0) {
			r.WriteByte(uCWvsAppUpdateTimeCall + 2, 0xEB);
		}
#pragma endregion
	}

	bool RemoveNewPatcherCheck(Rosemary& r)
	{
		// Description: Strip out the initialization of CRC for the current module and every DLL in the directory
		for (const auto& addr : AOB_Scan_DeleteNewPatcher_Addrs) {
			ULONG_PTR uAddr = r.Scan(addr);
			if (uAddr > 0) {
				return r.WriteCode(uAddr, AOB_Code_Ret);
			}
		}
		return false;
	}

	bool RemoveDisconnectException(Rosemary& r)
	{
		// Description: Skip AVCDisconnectException
		for (const auto& uCall : AOB_Scan_Disconnect_Exception_Calls)
		{
			ULONG_PTR uAddr = r.Scan(uCall);
			if (uAddr) {
				return r.WriteCode(uAddr, L"75 04 90 90 90 90");
			}
		}
		return false;
	}

	bool RemoveSecurityClient(Rosemary& r)
	{
		// CSecurityClient::CSecurityClient
		ULONG_PTR uAddr = r.Scan(AOB_Scan_CSecurityClient__Ctor_Addr);
		if (uAddr == 0) {
			DEBUG(L"Failed to find CSecurityClient::CSecurityClient");
			return false;
		}
		// ZExceptionHandler::ZExceptionHandler in CSecurityClient::CSecurityClient
		ULONG_PTR uExceptionHandlerAddr = GetFuncAddress(uAddr + 0x3A);
		if (uExceptionHandlerAddr == 0) {
			DEBUG(L"Failed to find ZExceptionHandler::ZExceptionHandler");
			return false;
		}
		r.WriteCode(uExceptionHandlerAddr, AOB_Code_Ret);
		// CSecurityClient::StartModule
		ULONG_PTR uStartModuleAddr = r.Scan(AOB_Scan_CSecurityClient__StartModule_Addr);
		if (uStartModuleAddr == 0) {
			DEBUG(L"Failed to find CSecurityClient::StartModule");
			return false;
		}
		r.WriteCode(uStartModuleAddr, L"31 C0 C2 04 00");
		// CSecurityClient::StartKeyCrypt(npkcrypt)
		ULONG_PTR uStartKeyCryptAddr = r.Scan(AOB_Scan_CSecurityClient__StartKeyCrypt_Addr);
		if (uStartKeyCryptAddr == 0) {
			DEBUG(L"Failed to find CSecurityClient::StartKeyCrypt");
			return false;
		}
		r.WriteCode(uStartKeyCryptAddr, AOB_Code_Ret);
		return true;
	}

	bool RemoveSecurityClientInitFailedException(Rosemary& r)
	{
		ULONG_PTR uAddr = 0;
		for (const std::wstring& AOB : AOB_Scan_CSecurityClient__InitFailed_Exception_Addr) {
			uAddr = r.Scan(AOB);
			if (uAddr == 0) {
				DEBUG(L"Failed to find AVCSecurityInitFailed exception");
				return false;
			}
			r.WriteCode(uAddr, AOB_Code_Ret);
		}
		return true;
	}

}