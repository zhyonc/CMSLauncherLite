#include "AOBScan.h"

AOBScan::AOBScan(std::wstring wAOB) {
	m_init = CreateAOB(wAOB);
}

bool AOBScan::CreateAOB(std::wstring wAOB) {
	if (wAOB.length() < 2) {
		return false;
	}

	std::wstring AOB;

	for (size_t i = 0; i < wAOB.length(); i++) {
		if (wAOB[i] == L' ') {
			continue;
		}
		if (L'0' <= wAOB[i] && wAOB[i] <= L'9') {
			AOB.push_back(wAOB[i]);
			continue;
		}
		if (L'A' <= wAOB[i] && wAOB[i] <= L'F') {
			AOB.push_back(wAOB[i]);
			continue;
		}
		if (L'a' <= wAOB[i] && wAOB[i] <= L'f') {
			AOB.push_back((unsigned short)wAOB[i] - 0x20);
			continue;
		}
		if (wAOB[i] == L'?' || wAOB[i] == L'*') {
			AOB.push_back(L'?');
			continue;
		}
		return false;
	}

	if (AOB.length() % 2) {
		return false;
	}

	for (size_t i = 0; i < AOB.length(); i += 2) {
		unsigned char t = 0x00;
		int m = 0;
		if (AOB[i] == L'?') {
			m |= 1;
		}
		if (L'0' <= AOB[i] && AOB[i] <= L'9') {
			t |= ((unsigned char)AOB[i] - 0x30) << 4;
		}
		else if (L'A' <= AOB[i] && AOB[i] <= L'F') {
			t |= ((unsigned char)AOB[i] - 0x37) << 4;
		}
		if (AOB[i + 1] == L'?') {
			m |= 2;
		}
		if (L'0' <= AOB[i + 1] && AOB[i + 1] <= L'9') {
			t |= ((unsigned char)AOB[i + 1] - 0x30);
		}
		else if (L'A' <= AOB[i + 1] && AOB[i + 1] <= L'F') {
			t |= ((unsigned char)AOB[i + 1] - 0x37);
		}

		m_array_of_bytes.push_back(t);
		m_mask.push_back(m);
	}

	return true;
}


bool AOBScan::Compare(unsigned __int64 uAddress) {
	if (!m_init) {
		return false;
	}

	try {
		for (size_t i = 0; i < m_array_of_bytes.size(); i++) {
			// ignore
			if (m_mask[i] == 3) {
				continue;
			}
			// compare
			if (m_mask[i] == 0 && ((unsigned char*)uAddress)[i] == m_array_of_bytes[i]) {
				continue;
			}
			// ignore high
			if (m_mask[i] == 1 && (((unsigned char*)uAddress)[i] & 0x0F) == (m_array_of_bytes[i] & 0x0F)) {
				continue;
			}
			// ignore low
			if (m_mask[i] == 2 && (((unsigned char*)uAddress)[i] & 0xF0) == (m_array_of_bytes[i] & 0xF0)) {
				continue;
			}

			return false;
		}
	}
	catch (...) {
		return false;
	}

	return true;
}

size_t AOBScan::size() {
	return m_array_of_bytes.size();
}