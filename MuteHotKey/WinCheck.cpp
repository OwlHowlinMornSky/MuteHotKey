﻿
#include "WinCheck.h"
#include "framework.h"

#include <string>

void ParseWindowsSystemError(std::wstring_view errorText) {
	DWORD lasterrcode = GetLastError();
	LPWSTR pBuffer = NULL;
	std::wstring msgstr_en, msgstr_user;
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, lasterrcode, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR)&pBuffer, 0, NULL);
	if (pBuffer) {
		pBuffer[lstrlenW(pBuffer) - 2] = '\0';
		msgstr_en.append(pBuffer);
		LocalFree(pBuffer);
	}
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, lasterrcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pBuffer, 0, NULL);
	if (pBuffer) {
		pBuffer[lstrlenW(pBuffer) - 2] = '\0';
		msgstr_user.append(pBuffer);
		LocalFree(pBuffer);
	}
	if ((!msgstr_user.empty()) && (msgstr_user != msgstr_en)) {
		msgstr_en.append(L"\r\n");
		msgstr_en.append(msgstr_user);
	}
	else if (msgstr_en.empty()) {
		msgstr_en.assign(L"NULL.");
	}
	MessageBoxW(NULL, msgstr_en.data(), errorText.data(), MB_ICONERROR);
	return;
}