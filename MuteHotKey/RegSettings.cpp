﻿
#include "RegSettings.h"

#include "framework.h"
#include <winreg.h>
#include "AppGlobal.h"
#include "WinCheck.h"

namespace {

bool g_startAtRun = false;
bool g_hideAfterStart = false;

bool LoadBoolFromReg(std::wstring_view name, bool& value) {
	DWORD type = REG_DWORD;
	DWORD data = {};
	DWORD dataSize = sizeof(data);
	HKEY key = {};
	LSTATUS status = {};
	status = RegCreateKeyExW(
		HKEY_CURRENT_USER,
		L"SOFTWARE\\OHMS\\MuteHotKey\\Settings",
		NULL,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&key,
		NULL
	);
	if (status != 0) {
		ParseErrorCode(status, AppNameW + L": Failed to Open Registry Key");
		return false;
	}
	status = RegGetValueW(
		key,
		NULL,
		name.data(),
		RRF_RT_REG_DWORD,
		&type,
		&data,
		&dataSize
	);
	bool success = false;
	switch (status) {
	case ERROR_SUCCESS:
		value = data;
		success = true;
		break;
	case ERROR_FILE_NOT_FOUND:
		type = REG_DWORD;
		data = 0;
		dataSize = sizeof(data);
		RegSetValueExW(
			key,
			name.data(),
			NULL,
			type,
			(PBYTE)&data,
			dataSize
		);
		value = false;
		success = true;
		break;
	case ERROR_MORE_DATA:
	case ERROR_INVALID_PARAMETER:
	default:
		ParseErrorCode(status, AppNameW + L": Failed to Read Registry Value \'" + name.data() + L"\'");
		break;
	}
	RegCloseKey(key);
	return success;
}

bool SaveBoolToReg(std::wstring_view name, bool value) {
	DWORD type = REG_DWORD;
	DWORD data = value;
	DWORD dataSize = sizeof(data);
	HKEY key = {};
	LSTATUS status = {};
	status = RegCreateKeyExW(
		HKEY_CURRENT_USER,
		L"SOFTWARE\\OHMS\\MuteHotKey\\Settings",
		NULL,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&key,
		NULL
	);
	if (status != 0) {
		ParseErrorCode(status, AppNameW + L": Failed to Open Registry Key");
		return false;
	}
	status = RegSetValueExW(
		key,
		name.data(),
		NULL,
		type,
		(PBYTE)&data,
		dataSize
	);
	RegCloseKey(key);
	return true;
}

}

bool LoadSettings() {
	if (!LoadBoolFromReg(L"StartAtRun", g_startAtRun))
		return false;
	if (!LoadBoolFromReg(L"HideAfterStart", g_hideAfterStart))
		return false;
	return true;
}

bool SaveSettings() {
	SaveBoolToReg(L"StartAtRun", g_startAtRun);
	SaveBoolToReg(L"HideAfterStart", g_hideAfterStart);
	return true;
}

bool GetStartAtRun() {
	return g_startAtRun;
}

void SetStartAtRun(bool startAtRun) {
	g_startAtRun = startAtRun;
}

bool GetHideAfterStart() {
	return g_hideAfterStart;
}

void SetHideAfterStart(bool hideAfterStart) {
	g_hideAfterStart = hideAfterStart;
}