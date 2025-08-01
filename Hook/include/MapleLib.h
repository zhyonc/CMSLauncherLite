#pragma once
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <tlhelp32.h>
#include <vector>
#include <string>

#pragma region AOBHelper
ULONG_PTR GetFuncAddress(ULONG_PTR uAddr);
ULONG_PTR GetDWORDAddress(ULONG_PTR uAddr);
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
#pragma endregion

#pragma region AOBScan
class AOBScan {
private:
	bool m_init;
	std::vector<unsigned char> m_array_of_bytes;
	std::vector<unsigned char> m_mask;
	bool CreateAOB(std::wstring wAOB);
public:
	AOBScan(std::wstring wAOB);
	bool Compare(unsigned __int64 uAddress);
	size_t size();
};
#pragma endregion

#pragma region Logger

#ifndef IS_DEBUG_MODE
#define IS_DEBUG_MODE true
#endif

#if IS_DEBUG_MODE
#define DEBUG(msg) \
	{\
		std::wstring wmsg = L"[Debug] ";\
		wmsg += msg;\
		OutputDebugStringW(wmsg.c_str());\
	}
#else
#define DEBUG(msg)
#endif

#define SCANRES(msg) { DEBUG(L""#msg" = " + DWORD2String(msg));}

#define CHECK_HRESULT(HR) \
do { \
    HRESULT _hr = (HR); \
    if (FAILED(_hr)) { \
		SCANRES(_hr); \
        _com_issue_error(_hr); \
    } \
} while (0)
#pragma endregion

#pragma region ProcessHelper
void GetModuleEntryList(std::vector<MODULEENTRY32W>& vModuleEntryList);
bool FindModuleEntry(const std::wstring& wModuleName, BYTE** ppBaseAddr, DWORD& baseSize);
bool IsModuleCalled(const std::wstring& wModuleName, void* pReturnAddress);
bool ExtractSectionList(const std::wstring& wModuleName, std::vector<MEMORY_BASIC_INFORMATION>& vTextSection, std::vector<MEMORY_BASIC_INFORMATION>& vDataSection);
#pragma endregion

#pragma region Rosemary
class Rosemary {
private:
	bool m_init;
	std::vector<MEMORY_BASIC_INFORMATION> m_text_section_list;
	std::vector<MEMORY_BASIC_INFORMATION> m_data_section_list;
public:
	Rosemary();
	~Rosemary();
	bool IsSectionInitialized() const;
	void InitSectionList(std::wstring wModuleName);
	void GetTextSectionList(std::vector<MEMORY_BASIC_INFORMATION>& vSection);
	void GetDataSectionList(std::vector<MEMORY_BASIC_INFORMATION>& vSection);
	ULONG_PTR Scan(std::wstring wAOB, int res = 0);
	ULONG_PTR Scan(std::wstring wAOBList[], size_t size, size_t& index, bool(*Scanner)(ULONG_PTR) = NULL);
	ULONG_PTR Scan(std::wstring wAOB, bool (*Scanner)(ULONG_PTR));
	bool WriteCode(ULONG_PTR uAddress, const std::wstring wCode);
	bool WriteByte(ULONG_PTR uAddress, const unsigned char ucValue);
	bool FillBytes(ULONG_PTR uAddress, const unsigned char ucValue, const int nCount);
	bool WriteShort(ULONG_PTR uAddress, const unsigned short usValue);
	bool WriteInt(ULONG_PTR uAddress, const unsigned int uiValue);
	bool WriteLong(ULONG_PTR uAddress, const unsigned long ulValue);
	bool WriteString(ULONG_PTR uAddress, const char* cValue);
	bool WriteFloat(ULONG_PTR uAddress, const float fValue);
	bool WriteDouble(ULONG_PTR uAddress, const double dValue);
	bool Backup(std::vector<MEMORY_BASIC_INFORMATION>& vSection, std::vector<void*>& vBackup);
	bool JMP(ULONG_PTR uPrev, ULONG_PTR uNext, ULONG_PTR uNop = 0);
	bool Hook(ULONG_PTR uAddress, void* HookFunction, ULONG_PTR uNop = 0);
	ULONG_PTR StringPatch(std::string search_string, std::string replace_string);
};
#pragma endregion

#pragma region StringHelper
std::wstring BYTE2String(BYTE b);
std::wstring DWORD2String(DWORD dw);
std::wstring Str2WStr(const std::string& str);
#pragma endregion