#include "Rosemary.h"
#include "AOBScan.h"

Rosemary::Rosemary() {
	m_init = false;
}

Rosemary::~Rosemary() {
	m_text_section_list.clear();
	m_data_section_list.clear();
}

bool Rosemary::IsSectionInitialized() const
{
	return m_init;
}

void Rosemary::InitSectionList(const std::vector<MODULEENTRY32W>* const gModuleEntryList, std::wstring wModuleName) {
	if (m_init) {
		return;
	}
	m_text_section_list.clear();
	m_data_section_list.clear();
	for (size_t i = 0; i < gModuleEntryList->size(); i++) {
		const MODULEENTRY32W& me32 = (*gModuleEntryList)[i];
		if (_wcsicmp(me32.szModule, wModuleName.c_str()) == 0) {
			MEMORY_BASIC_INFORMATION mbi;
			memset(&mbi, 0, sizeof(mbi));

			ULONG_PTR baseAddr = (ULONG_PTR)me32.modBaseAddr;
			ULONG_PTR endAddr = baseAddr + me32.modBaseSize;
			while (baseAddr < endAddr)
			{
				if (VirtualQuery((void*)baseAddr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
					// text section for aob scan
					if (mbi.Protect & (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)) {
						m_text_section_list.push_back(mbi);
					}
					// data section for string search
					if (m_text_section_list.size() && mbi.Protect & (PAGE_READWRITE | PAGE_WRITECOPY)) {
						// PAGE_READONLY and EXECUTE are currently ignored
						m_data_section_list.push_back(mbi);
					}
					baseAddr += mbi.RegionSize;
				}
			}

			if (!m_text_section_list.empty()) {
				m_init = true;
				return;
			}
		}
	}
}

void Rosemary::GetTextSectionList(std::vector<MEMORY_BASIC_INFORMATION>& vSection) {
	vSection = m_text_section_list;
}

void Rosemary::GetDataSectionList(std::vector<MEMORY_BASIC_INFORMATION>& vSection)
{
	vSection = m_data_section_list;
}

ULONG_PTR Rosemary::Scan(std::wstring wAOB, int res) {
	if (!m_init) {
		return 0;
	}

	AOBScan a(wAOB);

	int count = 0;
	for (size_t i = 0; i < m_text_section_list.size(); i++) {
		for (ULONG_PTR uAddress = (ULONG_PTR)m_text_section_list[i].BaseAddress; uAddress < ((ULONG_PTR)m_text_section_list[i].BaseAddress + m_text_section_list[i].RegionSize); uAddress++) {
			if (a.Compare(uAddress)) {
				count++;
				if (res) {
					if (res != count) {
						continue;
					}
				}
				return uAddress;
			}
		}
	}

	return 0;
}

// ListScan
ULONG_PTR Rosemary::Scan(std::wstring wAOBList[], size_t size, size_t& index, bool(*Scanner)(ULONG_PTR)) {
	ULONG_PTR uAddress = 0;

	index = -1;

	if (!m_init) {
		return 0;
	}

	for (size_t i = 0; i < size; i++) {

		if (!Scanner) {
			uAddress = Scan(wAOBList[i]);
		}
		else {
			uAddress = Scan(wAOBList[i], Scanner);
		}

		if (uAddress) {
			index = i;
			return uAddress;
		}
	}

	return 0;
}


ULONG_PTR Rosemary::Scan(std::wstring wAOB, bool(*Scanner)(ULONG_PTR)) {
	if (!m_init) {
		return 0;
	}

	if (!Scanner) {
		return 0;
	}

	AOBScan a(wAOB);

	for (size_t i = 0; i < m_text_section_list.size(); i++) {
		for (ULONG_PTR uAddress = (ULONG_PTR)m_text_section_list[i].BaseAddress; uAddress < ((ULONG_PTR)m_text_section_list[i].BaseAddress + m_text_section_list[i].RegionSize); uAddress++) {
			if (a.Compare(uAddress)) {
				if (Scanner(uAddress)) {
					return uAddress;
				}
			}
		}
	}

	return 0;
}

bool Rosemary::WriteCode(ULONG_PTR uAddress, const std::wstring wCode) {
	if (!uAddress) {
		return false;
	}
	if (wCode.length() < 2) {
		return false;
	}
	std::wstring Code;
	// Format wCode to Code
	for (size_t i = 0; i < wCode.length(); i++) {
		if (wCode[i] == L' ') {
			// Skip spaces
			continue;
		}
		if (L'0' <= wCode[i] && wCode[i] <= L'9') {
			Code.push_back(wCode[i]);
			continue;
		}
		if (L'A' <= wCode[i] && wCode[i] <= L'F') {
			Code.push_back(wCode[i]);
			continue;
		}
		if (L'a' <= wCode[i] && wCode[i] <= L'f') {
			// toupper
			Code.push_back((unsigned short)wCode[i] - 0x20);
			continue;
		}
		return false;
	}

	if (Code.length() % 2) {
		return false;
	}

	// Convert hex string to byte vector
	std::vector<unsigned char> code;
	for (size_t i = 0; i < Code.length(); i += 2) {
		unsigned char t = 0x00;
		if (L'0' <= Code[i] && Code[i] <= L'9') {
			t |= ((unsigned char)Code[i] - 0x30) << 4;
		}
		else if (L'A' <= Code[i] && Code[i] <= L'F') {
			t |= ((unsigned char)Code[i] - 0x37) << 4;
		}
		if (L'0' <= Code[i + 1] && Code[i + 1] <= L'9') {
			t |= ((unsigned char)Code[i + 1] - 0x30);
		}
		else if (L'A' <= Code[i + 1] && Code[i + 1] <= L'F') {
			t |= ((unsigned char)Code[i + 1] - 0x37);
		}

		code.push_back(t);
	}

	if (!code.size()) {
		return false;
	}

	DWORD oldProtect = 0;
	if (!VirtualProtect((void*)uAddress, code.size(), PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	for (size_t i = 0; i < code.size(); i++) {
		((BYTE*)uAddress)[i] = code[i];
	}

	VirtualProtect((void*)uAddress, code.size(), oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteByte(ULONG_PTR uAddress, const unsigned char ucValue) {
	if (!uAddress) {
		return false;
	}
	DWORD oldProtect = 0;
	size_t nSize = sizeof(ucValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(unsigned char*)uAddress = ucValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::FillBytes(ULONG_PTR uAddress, const unsigned char ucValue, const int nCount) {
	if (!uAddress) {
		return false;
	}
	DWORD oldProtect = 0;
	size_t nSize = sizeof(ucValue);
	if (!VirtualProtect((void*)uAddress, nCount, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	memset((void*)uAddress, ucValue, nCount);

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteShort(ULONG_PTR uAddress, const unsigned short usValue) {
	if (!uAddress) {
		return false;
	}
	DWORD oldProtect = 0;
	size_t nSize = sizeof(usValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(unsigned short*)uAddress = usValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteInt(ULONG_PTR uAddress, const unsigned int uiValue) {
	if (!uAddress) {
		return false;
	}
	DWORD oldProtect = 0;
	size_t nSize = sizeof(uiValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(unsigned int*)uAddress = uiValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteLong(ULONG_PTR uAddress, const unsigned long ulValue) {
	if (!uAddress) {
		return false;
	}
	DWORD oldProtect = 0;
	size_t nSize = sizeof(ulValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(unsigned long*)uAddress = ulValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteString(ULONG_PTR uAddress, const char* cValue) {
	if (!uAddress) {
		return false;
	}
	if (cValue == nullptr) {
		return false;
	}
	DWORD oldProtect = 0;
	size_t nSize = strlen(cValue) + 1;
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	memcpy((void*)uAddress, cValue, nSize);

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);
	return true;
}

bool Rosemary::WriteFloat(ULONG_PTR uAddress, const float fValue) {
	if (!uAddress) {
		return false;
	}
	DWORD oldProtect = 0;
	size_t nSize = sizeof(fValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(float*)uAddress = fValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::WriteDouble(ULONG_PTR uAddress, const double dValue) {
	if (!uAddress) {
		return false;
	}
	DWORD oldProtect = 0;
	size_t nSize = sizeof(dValue);
	if (!VirtualProtect((void*)uAddress, nSize, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(double*)uAddress = dValue;

	VirtualProtect((void*)uAddress, nSize, oldProtect, &oldProtect);

	return true;
}

bool Rosemary::Backup(std::vector<MEMORY_BASIC_INFORMATION>& vSection, std::vector<void*>& vBackup) {
	vSection.clear();
	vBackup.clear();

	if (!m_init) {
		return false;
	}

	for (size_t i = 0; i < m_text_section_list.size(); i++) {

		const auto& section = m_text_section_list[i];

		void* memory = VirtualAlloc(NULL, section.RegionSize, MEM_COMMIT, PAGE_READWRITE);
		if (!memory) {
			vBackup.clear();
			return false;
		}
		memcpy(memory, section.BaseAddress, section.RegionSize);
		vBackup.push_back(memory);
	}

	vSection = m_text_section_list;
	return true;
}

bool Rosemary::Hook(ULONG_PTR uAddress, void* HookFunction, ULONG_PTR uNop) {
	DWORD oldProtect = 0;
	if (!VirtualProtect((void*)uAddress, 5 + uNop, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(BYTE*)uAddress = 0xE9;
	*(DWORD*)(uAddress + 0x01) = (DWORD)((ULONG_PTR)HookFunction - uAddress) - 0x05;

	for (size_t i = 0; i < uNop; i++) {
		((BYTE*)uAddress)[5 + i] = 0x90;
	}

	VirtualProtect((void*)uAddress, 5 + uNop, oldProtect, &oldProtect);
	return true;
}

bool Rosemary::JMP(ULONG_PTR uPrev, ULONG_PTR uNext, ULONG_PTR uNop) {
	DWORD oldProtect = 0;
	if (!VirtualProtect((void*)uPrev, 5 + uNop, PAGE_EXECUTE_READWRITE, &oldProtect)) {
		return false;
	}

	*(BYTE*)uPrev = 0xE9;
	*(DWORD*)(uPrev + 0x01) = (DWORD)(uNext - uPrev) - 0x05;

	for (size_t i = 0; i < uNop; i++) {
		((BYTE*)uPrev)[5 + i] = 0x90;
	}

	VirtualProtect((void*)uPrev, 5 + uNop, oldProtect, &oldProtect);
	return true;
}

ULONG_PTR Rosemary::StringPatch(std::string search_string, std::string replace_string) {

	if (!m_init) {
		return 0;
	}

	// including null
	size_t search_string_size = search_string.length() + 1;
	size_t replace_string_size = replace_string.length() + 1;

	if (search_string_size < replace_string_size) {
		// unsafe
		return 0;
	}
	for (size_t i = 0; i < m_data_section_list.size(); i++) {
		for (ULONG_PTR uAddress = (ULONG_PTR)m_data_section_list[i].BaseAddress; uAddress < ((ULONG_PTR)m_data_section_list[i].BaseAddress + m_data_section_list[i].RegionSize - search_string_size); uAddress++) {
			if (memcmp((void*)uAddress, search_string.c_str(), search_string_size) == 0) {
				memset((void*)uAddress, 0, search_string_size);
				memcpy_s((void*)uAddress, replace_string_size, replace_string.c_str(), replace_string_size);
				return uAddress;
			}
		}
	}
	// some packer's has execute flags
	for (size_t i = 0; i < m_text_section_list.size(); i++) {
		for (ULONG_PTR uAddress = (ULONG_PTR)m_text_section_list[i].BaseAddress; uAddress < ((ULONG_PTR)m_text_section_list[i].BaseAddress + m_text_section_list[i].RegionSize - search_string_size); uAddress++) {
			if (memcmp((void*)uAddress, search_string.c_str(), search_string_size) == 0) {
				memset((void*)uAddress, 0, search_string_size);
				memcpy_s((void*)uAddress, replace_string_size, replace_string.c_str(), replace_string_size);
				return uAddress;
			}
		}
	}
	return 0;
}