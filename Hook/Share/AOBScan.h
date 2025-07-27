#pragma once
#include <vector>
#include <string>

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