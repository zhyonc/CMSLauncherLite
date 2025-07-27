#include "Injector.h"
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#define IS_DEV false
#define GAME_DIR L"H:\\Maple\\MapleClient\\CMS15"
#define GAME_NAME L"MapleStory.exe"
#define DLL_NAME L"Hook.dll"

bool PathExists(const std::wstring& path) {
	return PathFileExists(path.c_str()) == TRUE;
}

std::wstring GetExecutablePath() {
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	return std::wstring(path);
}

std::wstring GetExecutableDir() {
	std::wstring executablePath = GetExecutablePath();
	size_t pos = executablePath.find_last_of(L"\\/");
	return (std::wstring::npos == pos) ? L"" : executablePath.substr(0, pos);
}

int main() {
	FreeConsole();
	std::wstring exeDir;
	std::wstring wDllPath;
	std::wstring wTargetPath;
	exeDir = GetExecutableDir();
	wDllPath = exeDir + L"\\" + DLL_NAME;
#if IS_DEV
	wTargetPath = std::wstring(GAME_DIR) + L"\\" + GAME_NAME;
#else
	wTargetPath = exeDir + L"\\" + GAME_NAME;
	if (!PathExists(wTargetPath)) {
		MessageBox(NULL, L"Please run in MapleStory directory", L"Launcher", MB_OK | MB_ICONINFORMATION);
		return 1;
	}
	if (!PathExists(wDllPath)) {
		std::wstring msg = L"Can't find file " + std::wstring(DLL_NAME);
		MessageBox(NULL, msg.c_str(), L"Launcher", MB_OK | MB_ICONERROR);
		return 1;
	}
#endif
	Injector injector(wTargetPath, wDllPath);
	if (!injector.Run()) {
		MessageBox(NULL, L"Unable to inject dll into target", L"Launcher", MB_OK | MB_ICONERROR);
		return 1;
	};
	return 0;
}