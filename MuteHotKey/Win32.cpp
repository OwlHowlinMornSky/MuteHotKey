/*
*                              MuteHotKey
*
*      Copyright 2024  Tyler Parret True
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*
* @Authors
*    Tyler Parret True <mysteryworldgod@outlook.com><https://github.com/OwlHowlinMornSky>
*/
#include "Win32.h"

#include <windowsx.h>
#include <CommCtrl.h>
#include <shellapi.h>
#include <strsafe.h>
#include "Mute.h"
#include "resource.h"
#include "WinCheck.h"
#include "AppGlobal.h"
#include "RegSettings.h"
#include "ContextMenu.h"

namespace {

constexpr size_t MAX_LOADSTRING = 128;
WCHAR szTitle[MAX_LOADSTRING]; // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING]; // 主窗口类名

WCHAR szSTOP[MAX_LOADSTRING];
WCHAR szSTART[MAX_LOADSTRING];
WCHAR szStartAtRun[MAX_LOADSTRING];
WCHAR szHideAfterStart[MAX_LOADSTRING];

constexpr UINT MYWM_CALLBACK = WM_APP + 233;
constexpr int NOTIFYICON_ID = 233;

HFONT hFont = NULL;
HWND hBtnMain = NULL;
HWND hBtnSettingStartAtRun = NULL;
HWND hBtnSettingHideAfterStart = NULL;
HWND hHot = NULL;
bool btnIsStarted = false;

UINT ModFromCode(WORD wHotkey) {
	UINT res = 0;
	BYTE code = HIBYTE(wHotkey);
	if (code & HOTKEYF_SHIFT)
		res |= MOD_SHIFT;
	if (code & HOTKEYF_CONTROL)
		res |= MOD_CONTROL;
	if (code & HOTKEYF_ALT)
		res |= MOD_ALT;
	return res;
}

UINT VkFromCode(WORD wHotkey) {
	return LOBYTE(wHotkey);
}

void OnBtnMain_Clicked(HWND hWnd) {
	if (btnIsStarted) {
		if (UnregisterHotKey(hWnd, 999)) {
			SetWindowTextW(hBtnMain, szSTART);
			EnableWindow(hHot, TRUE);
			btnIsStarted = false;
		}
		else {
			ParseWin32Error(AppNameW + L": Failed to Unregister Hotkey");
		}
	}
	else {
		SetStartAtRun(Button_GetCheck(hBtnSettingStartAtRun) == BST_CHECKED);
		SetHideAfterStart(Button_GetCheck(hBtnSettingHideAfterStart) == BST_CHECKED);

		EnableWindow(hHot, FALSE);

		SetHotKeySettings((WORD)SendMessageW(hHot, HKM_GETHOTKEY, 0, 0));

		if (GetHotKeySettings() == 0) {
			MessageBoxW(hWnd, L"Please set a hotkey combination.", AppNameW.data(), MB_ICONINFORMATION);
			EnableWindow(hHot, TRUE);
			return;
		}

		if (RegisterHotKey(hWnd, 999, ModFromCode(GetHotKeySettings()) | MOD_NOREPEAT, VkFromCode(GetHotKeySettings()))) {
			SetWindowTextW(hBtnMain, szSTOP);
			if (GetHideAfterStart()) {
				SendMessageW(hWnd, WM_SYSCOMMAND, SC_MINIMIZE | HTCAPTION, NULL);
			}
			btnIsStarted = true;
		}
		else {
			EnableWindow(hHot, TRUE);
			ParseWin32Error(AppNameW + L": Failed to Register Hotkey");
		}
	}
	return;
}

/**
 * @brief 保底注销热键，同时这是读取设置的最后机会。
 */
void OnWindowDestroy(HWND hWnd) {
	if (UnregisterHotKey(hWnd, 999)) {
		SetWindowTextW(hBtnMain, szSTART);
		btnIsStarted = false;
	}
	SetStartAtRun(Button_GetCheck(hBtnSettingStartAtRun) == BST_CHECKED);
	SetHideAfterStart(Button_GetCheck(hBtnSettingHideAfterStart) == BST_CHECKED);
	SetHotKeySettings((WORD)SendMessageW(hHot, HKM_GETHOTKEY, 0, 0));
}

ContextMenu g_contextMenu;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		hFont = CreateFontW(
			0, 0, 0, 0, FW_DONTCARE,
			FALSE, FALSE, FALSE,
			GB2312_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			L"Segoe UI"
		);

		hBtnMain = CreateWindowExW(
			0,
			WC_BUTTONW, szSTART,
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			130, 60, 100, 40,
			hWnd, NULL, GetModuleHandleW(NULL), NULL
		);
		SendMessageW(hBtnMain, WM_SETFONT, (WPARAM)hFont, TRUE);

		hBtnSettingStartAtRun = CreateWindowExW(
			0,
			WC_BUTTONW, szStartAtRun,
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
			20, 50, 100, 30,
			hWnd, NULL, GetModuleHandleW(NULL), NULL
		);
		SendMessageW(hBtnSettingStartAtRun, WM_SETFONT, (WPARAM)hFont, TRUE);

		hBtnSettingHideAfterStart = CreateWindowExW(
			0,
			WC_BUTTONW, szHideAfterStart,
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
			20, 80, 100, 30,
			hWnd, NULL, GetModuleHandleW(NULL), NULL
		);
		SendMessageW(hBtnSettingHideAfterStart, WM_SETFONT, (WPARAM)hFont, TRUE);

		Button_SetCheck(hBtnSettingStartAtRun, GetStartAtRun() ? BST_CHECKED : BST_UNCHECKED);
		Button_SetCheck(hBtnSettingHideAfterStart, GetHideAfterStart() ? BST_CHECKED : BST_UNCHECKED);

		hHot = CreateWindowExW(
			0,                                     // no extended styles 
			HOTKEY_CLASSW,                         // class name 
			L"",                                   // no title (caption) 
			WS_TABSTOP | WS_CHILD | WS_VISIBLE,    // style 
			15, 10,                                // position 
			210, 30,                               // size 
			hWnd,                                  // parent window 
			NULL,                                  // uses class menu 
			GetModuleHandleW(NULL),                // instance 
			NULL                                   // no WM_CREATE parameter 
		);
		SendMessageW(hHot, WM_SETFONT, (WPARAM)hFont, TRUE);
		SendMessageW(hHot, HKM_SETHOTKEY, GetHotKeySettings(), NULL);
		SendMessageW(hHot, HKM_SETRULES, (WPARAM)HKCOMB_NONE, MAKELPARAM(HOTKEYF_ALT, 0));

		if (GetStartAtRun())
			OnBtnMain_Clicked(hWnd);
		break;

	case WM_DESTROY:
		OnWindowDestroy(hWnd);

		DestroyWindow(hHot);
		DestroyWindow(hBtnSettingHideAfterStart);
		DestroyWindow(hBtnSettingStartAtRun);
		DestroyWindow(hBtnMain);
		DeleteObject(hFont);
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			if ((HWND)lParam == hBtnMain) {
				OnBtnMain_Clicked(hWnd);
			}
			else if (LOWORD(wParam) == ContextMenu::Exit) {
				PostMessageW(hWnd, WM_CLOSE, NULL, NULL);
			}
		}
		break;

	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_MINIMIZE)
			ShowWindow(hWnd, SW_HIDE);
		else
			return DefWindowProcW(hWnd, message, wParam, lParam);
		break;

	case WM_HOTKEY:
		if (wParam == 999) {
			TryMute();
		}
		else {
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		break;

	case MYWM_CALLBACK:
		if (LOWORD(lParam) == NIN_SELECT)
			ShowWindow(hWnd, SW_RESTORE);
		else if (LOWORD(lParam) == WM_CONTEXTMENU)
			g_contextMenu.Pop(hWnd, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
		break;

	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return 0;
}

} // namespace

void MyLoadString(HINSTANCE hInst) {
	// 初始化字符串
	LoadStringW(hInst, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInst, IDC_MUTEHOTKEY, szWindowClass, MAX_LOADSTRING);

	LoadStringW(hInst, IDS_STOP, szSTOP, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_START, szSTART, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_STRING106, szStartAtRun, MAX_LOADSTRING);
	LoadStringW(hInst, IDS_STRING107, szHideAfterStart, MAX_LOADSTRING);

	AppNameW.assign(szTitle);

	CHAR szTitleA[MAX_LOADSTRING];
	LoadStringA(hInst, IDS_APP_TITLE, szTitleA, MAX_LOADSTRING);

	AppNameA.assign(szTitleA);
	return;
}

bool MyRegisterClass(HINSTANCE hInst) {
	WNDCLASSEXW wcex{ 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInst;
	wcex.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_MUTEHOTKEY));
	wcex.hIconSm = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_SMALL));
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszClassName = szWindowClass;
	return RegisterClassExW(&wcex);
}

HWND MyCreateWindow(HINSTANCE hInst, int nCmdShow) {
	HWND hWnd = CreateWindowExW(
		WS_EX_APPWINDOW,
		szWindowClass, szTitle,
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
		CW_USEDEFAULT, 0, 270, 170,
		NULL, NULL, hInst, NULL
	);
	if (!hWnd)
		return NULL;
	if (!(GetStartAtRun() && GetHideAfterStart())) {
		ShowWindow(hWnd, nCmdShow);
		UpdateWindow(hWnd);
	}
	return hWnd;
}

bool MyAddNotifyIcon(HINSTANCE hInst, HWND hWnd) {
	NOTIFYICONDATA nid{ 0 };
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uVersion = NOTIFYICON_VERSION_4;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_MESSAGE;
	nid.uCallbackMessage = MYWM_CALLBACK;
	nid.uID = NOTIFYICON_ID;
	// This text will be shown as the icon's tooltip.
	StringCchCopyW(nid.szTip, ARRAYSIZE(nid.szTip), szTitle);

	nid.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(IDI_SMALL));

	// Add the icon
	if (Shell_NotifyIconW(NIM_ADD, &nid) == FALSE)
		return false;
	// Set the version
	if (Shell_NotifyIconW(NIM_SETVERSION, &nid) == FALSE)
		return false;
	return true;
}

void MyRemoveNotifyIcon() {
	NOTIFYICONDATA nid{ 0 };
	nid.cbSize = sizeof(nid);
	nid.uID = NOTIFYICON_ID;
	Shell_NotifyIconW(NIM_DELETE, &nid);
	return;
}
