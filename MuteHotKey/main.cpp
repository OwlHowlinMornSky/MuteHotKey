
#include "framework.h"
#include "Resource.h"
#include "Win32.h"
#include "AppGlobal.h"
#include "WinCheck.h"
#include "RegSettings.h"

#include <combaseapi.h>

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow
) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MyLoadString(hInstance);
	LoadSettings();

	HRESULT hr = {};
	//hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); // Thread Without UI
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // Thread With UI
	switch (hr) {
	case S_OK:
	case S_FALSE:
		break;
	case RPC_E_CHANGED_MODE:
		ParseErrorCode(hr, AppNameW + L": COM Warning");
		break;
	case E_INVALIDARG:
	case E_OUTOFMEMORY:
	case E_UNEXPECTED:
	default:
		ParseErrorCode(hr, AppNameW + L": Failed to Initialize COM");
		return 1;
	}

	if (!MyRegisterClass(hInstance)) {
		ParseWin32Error(AppNameW + L": Failed to Register Window Class");
		return 1;
	}

	HWND hwnd = MyCreateWindow(hInstance, nCmdShow);
	if (hwnd == NULL) {
		ParseWin32Error(AppNameW + L": Failed to Create Window");
		return 1;
	}

	if (!MyAddNotifyIcon(hInstance, hwnd)) {
		MessageBoxW(NULL, L"Failed to Create Notify Icon!", AppNameW.data(), MB_ICONERROR);
		return 1;
	}

	MSG msg = {};
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	DestroyWindow(hwnd);

	MyRemoveNotifyIcon();

	CoUninitialize();

	SaveSettings();

	return 0;
}
