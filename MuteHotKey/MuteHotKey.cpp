
#include "framework.h"
#include "Resource.h"
#include "Win32.h"

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

	HRESULT hr = {};
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); // Thread With UI
	//hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); // Thread Without UI
	switch (hr) {
	case S_OK:
	case S_FALSE:
	case RPC_E_CHANGED_MODE:
		break;
	default:
		MessageBoxW(NULL, L"Failed to initialize COM!", L"MuteHotKey", MB_ICONERROR);
		return 1;
	}

	if (!MyRegisterClass(hInstance)) {
		MessageBoxW(NULL, L"Failed to register window class!", L"MuteHotKey", MB_ICONERROR);
		return 1;
	}

	HWND hwnd = MyCreateWindow(hInstance, nCmdShow);
	if (hwnd == NULL) {
		MessageBoxW(NULL, L"Failed to create window!", L"MuteHotKey", MB_ICONERROR);
		return 1;
	}

	if (!MyAddNotifyIcon(hInstance, hwnd)) {
		MessageBoxW(NULL, L"Failed to create notify icon!", L"MuteHotKey", MB_ICONERROR);
		return 1;
	}

	MSG msg = {};
	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	MyRemoveNotifyIcon();

	CoUninitialize();

	return 0;
}
