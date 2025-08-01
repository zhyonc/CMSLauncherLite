#include "pch.h"
#include "ResMan.h"

#include "Resources/AOBList.h"

namespace {

	static void** gResMan;				// IWzResMan
	static void** gRoot;				// IWzNameSpace
	static void** gSetRootNameSpaceFn;	// _PcSetRootNameSpace

	enum RESMAN_PARAM {
		RC_AUTO_SERIALIZE = 0x1,
		RC_AUTO_SERIALIZE_NO_CACHE = 0x2,
		RC_NO_AUTO_SERIALIZE = 0x4,
		RC_DEFAULT_AUTO_SERIALIZE = 0x0,
		RC_AUTO_SERIALIZE_MASK = 0x7,
		RC_AUTO_REPARSE = 0x10,
		RC_NO_AUTO_REPARSE = 0x20,
		RC_DEFAULT_AUTO_REPARSE = 0x0,
		RC_AUTO_REPARSE_MASK = 0x30,
	};

	// GMS95 void __cdecl PcCreateObject::IWzResMan(const wchar_t* sUOL,_com_ptr_t* pObj,IUnknown* pUnkOuter)
	void(__cdecl* _PcCreateObject__IWzResMan)(const wchar_t* sUOL, void* pObj, void* pUnkOuter) = nullptr;

	// GMS95 HRESULT __thiscall IWzResMan::SetResManParam(IWzResMan *this,RESMAN_PARAM nParam,int nRetainTime,int nNameSpaceCacheTime)
	HRESULT(__stdcall* _IWzResMan__SetResManParam)(void* ecx, RESMAN_PARAM nParam, int nRetainTime, int nNameSpaceCacheTime) = nullptr;

	// GMS95 HRESULT __cdecl PcSetRootNameSpace(IUnknown *pNameSpace)
	HRESULT(__cdecl* _PcSetRootNameSpace)(void* ecx, int mode) = nullptr;

	// GMS95 void __cdecl PcCreateObject::IWzNameSpace(const wchar_t* sUOL, _com_ptr_t* pObj, IUnknown* pUnkOuter)
	void(__cdecl* _PcCreateObject__IWzNameSpace)(const wchar_t* sUOL, void* pObj, void* pUnkOuter) = nullptr;

	// GMS95 void __thiscall Ztl_bstr_t::Ztl_bstr_t(Ztl_bstr_t *this, const wchar_t *s)
	void* (__thiscall* _bstr_ctor)(void* ecx, const wchar_t* s) = nullptr;

	// GMS95 void __cdecl PcCreateObject::IWzFileSystem(const wchar_t* sUOL, _com_ptr_t* pObj, IUnknown* pUnkOuter)
	void(__cdecl* _PcCreateObject__IWzFileSystem)(const wchar_t* sUOL, void* pObj, void* pUnkOuter) = nullptr;

	// GMS95 HRESULT __thiscall IWzFileSystem::Init(IWzFileSystem *this, Ztl_bstr_t sPath)
	HRESULT(__thiscall* _IWzFileSystem__Init)(void* ecx, void* sPath) = nullptr;

	// GMS95 HRESULT __thiscall IWzNameSpace::Mount(IWzNameSpace *this, Ztl_bstr_t sPath, IWzNameSpace *pDown, int nPriority)
	HRESULT(__thiscall* _IWzNameSpace__Mount)(void* ecx, void* sPath, void* pDown, int nPriority) = nullptr;

	void CWvsApp__Dir_SlashToBackSlash(char* sDir) {
		if (sDir == nullptr) {
			return;
		}
		size_t length = strlen(sDir);
		for (size_t i = 0; i < length; ++i) {
			if (sDir[i] == '\\') {
				sDir[i] = '/';
			}
		}
	}

	void CWvsApp__Dir_upDir(char* sDir) {
		if (sDir == nullptr) {
			return;
		}
		size_t length = strlen(sDir);
		if (length > 0 && sDir[length - 1] == '/') {
			sDir[length - 1] = '\0';
		}
		size_t i = length - 1;
		if (i > 0) {
			while (sDir[i] != '/') {
				if (--i <= 0) {
					return;
				}
			}
			sDir[i] = '\0';
		}
	}

	// GMS95 void __thiscall CWvsApp::InitializeResMan(CWvsApp *this)
	void(__thiscall* _CWvsApp__InitializeResMan)(void* ecx) = nullptr;
	void __fastcall CWvsApp__InitializeResMan_Hook(void* ecx, void* edx) {
		// PcCreateObject::IWzResMan
		_PcCreateObject__IWzResMan(L"ResMan", gResMan, nullptr);
		// IWzResMan::SetResManParam
		ULONG_PTR uSetResManParamCall = *(ULONG_PTR*)(*gResMan) + 20; // This is function ptr ptr
		ULONG_PTR uSetResManParamAddr = *(ULONG_PTR*)uSetResManParamCall;
		SADDR(&_IWzResMan__SetResManParam, uSetResManParamAddr);
		_IWzResMan__SetResManParam(*gResMan, (RESMAN_PARAM)(RC_AUTO_REPARSE | RC_AUTO_SERIALIZE), -1, -1);
		// PcSetRootNameSpace
		ULONG_PTR uSetRootNameSpaceAddr = *(ULONG_PTR*)gSetRootNameSpaceFn; // This is a function ptr
		SADDR(&_PcSetRootNameSpace, uSetRootNameSpaceAddr);
		// PcCreateObject::IWzNameSpace
		_PcCreateObject__IWzNameSpace(L"NameSpace", gRoot, nullptr);
		_PcSetRootNameSpace(gRoot, 1);
		// Get the current dir of exe
		static char sStartPath[MAX_PATH];
		GetModuleFileNameA(NULL, sStartPath, MAX_PATH);
		CWvsApp__Dir_SlashToBackSlash(sStartPath);
		CWvsApp__Dir_upDir(sStartPath);
		// Create FileSystem 
		void* pFileSystem = nullptr;
		_PcCreateObject__IWzFileSystem(L"NameSpace#FileSystem", &pFileSystem, nullptr);
		// Mount Game FileSystem
		void* sDataPath = nullptr;
		strcat_s(sStartPath, "/Data");
		std::wstring wsDataPath(sStartPath, sStartPath + strlen(sStartPath));
		_bstr_ctor(&sDataPath, wsDataPath.c_str());
		_IWzFileSystem__Init(pFileSystem, sDataPath);
		_bstr_ctor(&sDataPath, L"/");
		_IWzNameSpace__Mount(*gRoot, sDataPath, pFileSystem, 0);
		DEBUG(L"Mount ok");
	}

}

namespace ResMan {

	bool getFuncAddressWithOffset(ULONG_PTR uBaseAddr, const std::vector<short>& offsets, ULONG_PTR& uCall, ULONG_PTR& uAddr) {
		for (short offset : offsets) {
			uCall = uBaseAddr + offset;
			uAddr = GetFuncAddress(uCall);
			if (uAddr > 0) {
				return true;
			}
		}
		return false;
	}

	bool Mount(Rosemary& r) {
		// Check data directory
		DWORD attributes = GetFileAttributesW(L"Data");
		if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) {
			DEBUG(L"Failed to find Data directory");
			return false;
		}
		// CWvsApp::InitializeResMan base address
		ULONG_PTR uBaseAddr = r.Scan(AOB_Scan_CWvsApp__InitializeResMan_Addr);
		if (uBaseAddr == 0) {
			DEBUG(L"Failed to find CWvsApp::InitializeResMan");
			return false;
		}
		// PcCreateObject::IWzResMan
		ULONG_PTR uIWzResManCall = 0;
		ULONG_PTR uIWzResManAddr = 0;
		bool ok = getFuncAddressWithOffset(uBaseAddr, PcCreateObject__IWzResMan_Offsets, uIWzResManCall, uIWzResManAddr);
		if (!ok) {
			DEBUG(L"Failed to find PcCreateObject::IWzResMan");
			return false;
		}
		SADDR(&_PcCreateObject__IWzResMan, uIWzResManAddr);
		// PcCreateObject::IWzNameSpace
		ULONG_PTR uIWzNameSpaceCall = 0;
		ULONG_PTR uIWzNameSpaceAddr = 0;
		ok = getFuncAddressWithOffset(uBaseAddr, PcCreateObject__IWzNameSpace_Offsets, uIWzNameSpaceCall, uIWzNameSpaceAddr);
		if (!ok) {
			DEBUG(L"Failed to find PcCreateObject::IWzNameSpace");
			return false;
		}
		SADDR(&_PcCreateObject__IWzNameSpace, uIWzNameSpaceAddr);
		// PcCreateObject::IWzFileSystem
		ULONG_PTR uIWzFileSystemCall = 0;
		ULONG_PTR uIWzFileSystemAddr = 0;
		ok = getFuncAddressWithOffset(uBaseAddr, PcCreateObject__IWzFileSystem_Offsets, uIWzFileSystemCall, uIWzFileSystemAddr);
		if (uIWzFileSystemAddr == 0) {
			DEBUG(L"Failed to find PcCreateObject::IWzFileSystem");
			return false;
		}
		SADDR(&_PcCreateObject__IWzFileSystem, uIWzFileSystemAddr);
		// _bstr_t::_bstr_t
		ULONG_PTR bstrCall = 0;
		ULONG_PTR bstrAddr = 0;
		ok = getFuncAddressWithOffset(uBaseAddr, bstr_ctor_Offsets, bstrCall, bstrAddr);
		if (bstrAddr == 0) {
			DEBUG(L"Failed to find _bstr_t::_bstr_t");
			return false;
		}
		SADDR(&_bstr_ctor, bstrAddr);
		// IWzFileSystem::Init
		ULONG_PTR uIWzFileSystemInitCall = 0;
		ULONG_PTR uIWzFileSystemInitAddr = 0;
		ok = getFuncAddressWithOffset(uBaseAddr, IWzFileSystem__Init_Offsets, uIWzFileSystemInitCall, uIWzFileSystemInitAddr);
		if (!ok) {
			DEBUG(L"Failed to find IWzFileSystem::Init");
			return false;
		}
		SADDR(&_IWzFileSystem__Init, uIWzFileSystemInitAddr);
		// IWzNameSpace::Mount
		ULONG_PTR uMountCall = 0;
		ULONG_PTR uMountAddr = 0;
		ok = getFuncAddressWithOffset(uBaseAddr, IWzFileSystem__Mount_Offsets, uMountCall, uMountAddr);
		if (!ok) {
			DEBUG(L"Failed to find IWzNameSpace::Mount");
			return false;
		}
		SADDR(&_IWzNameSpace__Mount, uMountAddr);
		// Global Var
		gResMan = (void**)(GetDWORDAddress(uIWzResManCall - 0x9));
		gRoot = (void**)(GetDWORDAddress(uIWzNameSpaceCall - 0x9));
		gSetRootNameSpaceFn = (void**)(GetFuncAddress(uIWzNameSpaceCall + 0x2A));
		// ResMan Ready
		SADDR(&_CWvsApp__InitializeResMan, uBaseAddr);
		return SHOOK(true, &_CWvsApp__InitializeResMan, CWvsApp__InitializeResMan_Hook);
	}

}