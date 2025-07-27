#pragma once
#include <string>
#include <vector>

#pragma region AntiCheat
#pragma region Common
const std::vector<std::wstring> AOB_Scan_DeleteNewPatcher_Addrs = {
	L"56 8B F1 E8 ?? ?? 00 00 8B CE E8 ?? ?? 00 00 5E C3", // CMS15(004E1907)
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 51 51 53 56 57 33 FF BE", // CMS27(005DF39E)-CMS29(005E54DC)-CMS35(006024FB)-CMS48(006C5FB0)-CMS59(0074B7F3)-CMS69(008C2FF9)
};
const std::vector<std::wstring> AOB_Scan_Disconnect_Exception_Calls = {
	L"0F 85 ?? ?? 00 00 39 1F 0F 85",	 // CMS15(0059223D)-CMS48(006C79A1)-CMS59(0074D850)-CMS69(008C57D8)
	L"0F 85 ?? ?? 00 00 8B 45 08 39 18", // CMS29(005E6DE8)-CMS35(00603E1E)
};
#pragma endregion

#pragma region CMS15
const std::wstring AOB_Scan_MapleStoryAcGuardian_Activation_Addr = L"74 ?? 8D ?? ?? 50 33 DB 56 89"; // CMS15(0x00590C3A)
const std::wstring AOB_Scan_CWvsApp__InitializeResMan_Integrity_Check_Addr = L"74 0A 8B ?? ?? C7"; // CMS15(0x00592F4A)
const std::wstring AOB_Scan_CWvsApp__InitAcGuardian_Addr = L"55 8B EC 81 EC B0 03 00 00 56"; // CMS15(0x00592778)
const std::wstring AOB_Scan_CSecurityThread__Start_Addr = L"B8 B8 2D 5D 00 E8"; // CMS15(0x00593833)
const std::wstring AOB_Scan_CWvsApp__UpdateTime_Call = L"85 D2 75 07 8B CE E8"; // CMS15(0x00593B47)
#pragma endregion

#pragma region CMS48-CMS69
const std::wstring AOB_Scan_CSecurityClient__Ctor_Addr = L"1B C0 23 C2 57 89 75 F0 A3 ?? ?? ?? ?? 33 FF 57"; // CMS48(006F1C5E)-CMS59(0078221B)-CMS69(0090C423)
const std::wstring AOB_Scan_CSecurityClient__StartModule_Addr = L"55 8B EC 81 EC 14 01 00 00 57 FF 75"; // CMS48(006F1D80)-CMS59(00782340)-CMS69(0090C548)
const std::wstring AOB_Scan_CSecurityClient__StartKeyCrypt_Addr = L"B8 ?? ?? ?? 00 E8 ?? ?? ?? 00 81 EC 10 02 00 00"; // CMS48(006F22EC)-CMS59(007828AC)-CMS69(0090CAC1)
const std::vector<std::wstring> AOB_Scan_CSecurityClient__InitFailed_Exception_Addr = {
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 51 51 E8 ?? ?? ?? ?? 3D 55 07 00", // CMS48(006F1D43)-CMS59(00782303)-CMS69(0090C50B)
	L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 51 51 56 8B F1 57 8D 7E 04", // CMS48(006F1E82)-CMS59(00782442)-CMS69(0090C64A)
};
#pragma endregion

#pragma endregion

#pragma region ResMan
// CWvsApp::InitializeResMan: CMS15(00592A42)-CMS27(005E1425)-CMS29(005E75DD)-CMS35(006045D9)-CMS48(006C809C)
const std::wstring AOB_Scan_CWvsApp__InitializeResMan_Addr = L"B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 81 EC ?? ?? 00 00 53 56 57 89 65 F0 68";
const std::vector<short> PcCreateObject__IWzResMan_Offsets = {
	0x3C,  // CMS15(00592A7E)
	0x39,  // CMS27(005E145E)-CMS29(005E7616)-CMS35(00604612)-CMS48(006C80D5)
};
const std::vector<short> PcCreateObject__IWzNameSpace_Offsets = {
	0xAC,  // CMS15(00592AEE)
	0xA9,  // CMS27(005E14CE)-CMS29(005E7686)-CMS35(00604682)-CMS48(006C8145)
};
const std::vector<short> PcCreateObject__IWzFileSystem_Offsets = {
	0x104, // CMS15(00592B46)
	0x101, // CMS27(005E1526)-CMS29(005E76DE)-CMS35(006046DA)-CMS48(006C819D)
};
const std::vector<short> bstr_ctor_Offsets = {
	0x320, // CMS15(00592D62)
	0x329, // CMS27(005E174E)-CMS29(005E7906)
	0x253, // CMS35(0060482C)-CMS48(006C82EF)
};
const std::vector<short> IWzFileSystem__Init_Offsets = {
	0X176, // CMS35(0060474F)-CMS48(006C8212)
	0x255, // CMS27(005E167A)-CMS29(005E7832) conflict with CMS48
	0x24C, // CMS15(00592C8E) conflict with CMS27/CMS29
};
const std::vector<short> IWzFileSystem__Mount_Offsets = {
	0x375, // CMS27(005E179A)-CMS29(005E7952)
	0x36C, // CMS15(00592DAE) conflict with CMS27/CMS29
	0x29F, // CMS35(00604878)-CMS48(006C833B) conflict with CMS15
};
#pragma endregion

#pragma region Wnd
const std::wstring AOB_Scan_CLogo_Init_Addr = L"85 C0 74 17 80 38 00 74 12"; // CMS15(0048FB26)-CMS29(004B2EE1)-CMS35(004B9454)-CMS48(00510A1B)-CMS59(00549813)-CMS69(005E4680)
const std::wstring AOB_Scan_Window_Mode_Addr = L"C7 45 E4 10 00 00 00 E8"; // CMS15(0059303B)-CMS29(005E7A52)-CMS35(00604A97)-CMS48(006C855A)-CMS59(0074E819)-CMS69(008C674E)
const std::wstring AOB_Scan_SetCooperativeLevel_Addr = L"8B 07 8B 08 6A 05"; // CMS15(0047062B)-CMS29(004904A4)-CMS35(004937F2)
const std::wstring AOB_Scan_CWndMan__TranslateMessage_Addr = L"55 8B EC 83 EC 24 53 8B 5D 08"; // CMS15(0058C3E2)-CMS29(005E0A37)-CMS35(005FDA2F)
const int CInputSystem__Instance_Offset = 0x16; // CWndMan::TranslateMessage base addr + 0x16
#pragma endregion

#pragma region Code
const std::wstring AOB_Code_Ret = L"31 C0 C3"; // xor eax,eax ret
const std::wstring AOB_Code_Ret1 = L"B8 01 00 00 00 C3"; // mov eax, 1 retn
#pragma endregion