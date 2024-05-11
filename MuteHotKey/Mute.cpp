
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <combaseapi.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <audioclient.h>

//#include <guiddef.h>
//#include <initguid.h>
#pragma comment(lib,"Strmiids.lib") 

#include <iostream>

static const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
static const IID IID_IAudioSessionManager2 = __uuidof(IAudioSessionManager2);

DWORD pid = NULL;

void DoSessionCtrl(IAudioSessionControl* pSessionCtrl) {
	HRESULT hr = {};
	IAudioSessionControl2* ctrl = nullptr;
	hr = pSessionCtrl->QueryInterface<IAudioSessionControl2>(&ctrl);
	if (hr != S_OK) {
		return;
	}

	if (ctrl->IsSystemSoundsSession() == S_OK) {
		return;
	}

	ISimpleAudioVolume* vol = nullptr;
	hr = pSessionCtrl->QueryInterface<ISimpleAudioVolume>(&vol);
	if (hr != S_OK) {
		//std::cout << "<Failed to Read>.";
		return;
	}
	//float volume = 0.0f;
	//hr = vol->GetMasterVolume(&volume);
	//if (hr == S_OK) {
	//	std::cout << "vol: " << volume << '.';
	//}
	//else {
	//	std::cout << "vol: <Failed to Get>.";
	//}

	DWORD apid = NULL;
	if (ctrl->GetProcessId(&apid) == S_OK) {
		//std::cout << " PID: " << apid;
		if (apid == pid) {
			BOOL muted = {};
			if (vol->GetMute(&muted) == S_OK) {
				if (vol->SetMute(!muted, NULL) != S_OK) {
					//std::cout << "Failed";
					;
				}
			}
		}
	}
	std::cout << std::endl;
	return;
}

void DeviceEnumSession(IMMDevice* pDevice) {
	HRESULT hr = {};
	IAudioSessionManager2* pSessionMngr = nullptr;
	hr = pDevice->Activate(IID_IAudioSessionManager2, CLSCTX_ALL, NULL, (void**)&pSessionMngr);
	if (hr != S_OK) {
		//std::cout << "DoDevice failed!" << std::endl;
		return;
	}
	IAudioSessionEnumerator* pSessionEnum = nullptr;
	hr = pSessionMngr->GetSessionEnumerator(&pSessionEnum);
	if (hr == S_OK) {
		int cnt = 0;
		hr = pSessionEnum->GetCount(&cnt);
		if (hr == S_OK) {
			for (int i = 0; i < cnt; ++i) {
				IAudioSessionControl* pSessionCtrl = nullptr;
				hr = pSessionEnum->GetSession(i, &pSessionCtrl);
				if (hr != S_OK)
					continue;
				DoSessionCtrl(pSessionCtrl);
				pSessionCtrl->Release();
			}
		}
		pSessionEnum->Release();
	}
	//else {
	//	std::cout << "Failed: GetSessionEnumerator()." << std::endl;
	//}
	pSessionMngr->Release();
}

void EnumDevice() {
	HRESULT hr = {};
	IMMDeviceEnumerator* pDevEnum = nullptr;
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pDevEnum);
	if (hr != S_OK) {
		//std::cout << "Failed: CoCreateInstance()." << std::endl;
		return;
	}

	IMMDeviceCollection* pDevCol = nullptr;
	hr = pDevEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevCol);
	if (hr == S_OK) {
		UINT cnt = 0;
		hr = pDevCol->GetCount(&cnt);
		if (hr == S_OK) {
			for (UINT i = 0; i < cnt; ++i) {
				IMMDevice* pDevice = nullptr;
				hr = pDevCol->Item(i, &pDevice);
				if (hr != S_OK) {
					//std::cout << "Failed: Item()." << std::endl;
					continue;
				}
				DeviceEnumSession(pDevice);
				pDevice->Release();
			}
		}
		//else {
		//	std::cout << "Failed: GetCount()." << std::endl;
		//}
		pDevCol->Release();
	}
	//else {
	//	std::cout << "Failed: EnumAudioEndpoints()." << std::endl;
	//}

	pDevEnum->Release();
	return;
}

void TryMute() {

	HWND hwnd = GetForegroundWindow();

	if (hwnd == NULL) {
		return;
	}

	pid = NULL;

	if (GetWindowThreadProcessId(hwnd, &pid) == 0) {
		return;
	}

	EnumDevice();

	return;
}
