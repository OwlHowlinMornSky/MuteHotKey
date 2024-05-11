#include "Win32.h"

#include <windowsx.h>
#include <CommCtrl.h>
#include <shellapi.h>
#include <strsafe.h>
#include "Mute.h"
#include "resource.h"
#include "WinCheck.h"

namespace {

constexpr size_t MAX_LOADSTRING = 128;
WCHAR szTitle[MAX_LOADSTRING]; // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING]; // 主窗口类名

WCHAR szSTOP[MAX_LOADSTRING];
WCHAR szSTART[MAX_LOADSTRING];

constexpr UINT MYWM_CALLBACK = WM_APP + 233;
constexpr int NOTIFYICON_ID = 233;

HFONT hFont = NULL;
HWND hBtnMain = NULL;
bool btnIsStarted = false;

void OnBtnMain_Clicked(HWND hWnd) {
	if (btnIsStarted) {
		if (UnregisterHotKey(hWnd, 999)) {
			SetWindowTextW(hBtnMain, szSTART);
			btnIsStarted = false;
		}
		else {
			ParseWindowsSystemError(L"MuteHotKey: Failed to Unregister Hotkey");
		}
	}
	else {
		if (RegisterHotKey(hWnd, 999, MOD_CONTROL | MOD_NOREPEAT, VK_OEM_COMMA)) {
			SetWindowTextW(hBtnMain, szSTOP);
			btnIsStarted = true;
		}
		else {
			ParseWindowsSystemError(L"MuteHotKey: Failed to Register Hotkey");
		}
	}
	return;
}

/**
 * @brief 尝试注销热键（保底）。
 */
void TryRemoveHotKey(HWND hWnd) {
	if (UnregisterHotKey(hWnd, 999)) {
		SetWindowTextW(hBtnMain, szSTART);
		btnIsStarted = false;
	}
}

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

		hBtnMain = CreateWindowW(
			WC_BUTTONW, szSTART,
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
			81, 40, 100, 40,
			hWnd, NULL, GetModuleHandleW(NULL), NULL
		);
		SendMessageW(hBtnMain, WM_SETFONT, (WPARAM)hFont, TRUE);
		break;

	case WM_DESTROY:
		TryRemoveHotKey(hWnd); // 保底注销热键

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
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
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
