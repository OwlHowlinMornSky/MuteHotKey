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
#include "framework.h"

#include <combaseapi.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <audioclient.h>

#pragma comment(lib,"Strmiids.lib") 

#include <exception>
#include "AppGlobal.h"
#include "Mute.h"

namespace {

static const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
static const IID IID_IAudioSessionManager2 = __uuidof(IAudioSessionManager2);

DWORD pid = NULL;

void DoSessionCtrl(IAudioSessionControl* pSessionCtrl) {
	HRESULT hr = {};
	IAudioSessionControl2* ctrl = nullptr;
	hr = pSessionCtrl->QueryInterface<IAudioSessionControl2>(&ctrl);
	if (hr != S_OK)
		return;
	if (ctrl->IsSystemSoundsSession() == S_OK) // 不操作系统会话
		return;
	ISimpleAudioVolume* vol = nullptr;
	hr = pSessionCtrl->QueryInterface<ISimpleAudioVolume>(&vol);
	if (hr != S_OK)
		return;
	DWORD apid = NULL;
	if (ctrl->GetProcessId(&apid) != S_OK)
		return;
	if (apid != pid)
		return;
	BOOL muted = {};
	if (vol->GetMute(&muted) != S_OK)
		return;
	vol->SetMute(!muted, NULL);
	return;
}

void DeviceEnumSession(IMMDevice* pDevice) {
	HRESULT hr = {};
	IAudioSessionManager2* pSessionMngr = nullptr;
	hr = pDevice->Activate(IID_IAudioSessionManager2, CLSCTX_ALL, NULL, (void**)&pSessionMngr);
	if (hr != S_OK)
		return;
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
	pSessionMngr->Release();
}

void EnumDevice() {
	HRESULT hr = {};
	IMMDeviceEnumerator* pDevEnum = nullptr;
	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pDevEnum);
	if (hr != S_OK)
		return;
	IMMDeviceCollection* pDevCol = nullptr;
	hr = pDevEnum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDevCol);
	if (hr == S_OK) {
		UINT cnt = 0;
		hr = pDevCol->GetCount(&cnt);
		if (hr == S_OK) {
			for (UINT i = 0; i < cnt; ++i) {
				IMMDevice* pDevice = nullptr;
				hr = pDevCol->Item(i, &pDevice);
				if (hr != S_OK)
					continue;
				DeviceEnumSession(pDevice);
				pDevice->Release();
			}
		}
		pDevCol->Release();
	}
	pDevEnum->Release();
	return;
}

} // namespace

bool InitCOM() {
    return false;
}

void TryMute() {
	HWND hwnd = GetForegroundWindow();
	if (hwnd == NULL)
		return;
	pid = NULL;
	if (GetWindowThreadProcessId(hwnd, &pid) == 0)
		return;
	try {
		EnumDevice();
	}
	catch (std::exception& e) {
		MessageBoxA(NULL, e.what(), (AppNameA + ": Task Exception").data(), MB_ICONERROR);
	}
	catch (...) {
		MessageBoxA(NULL, "Unknown Exception.", (AppNameA + ": Task Exception").data(), MB_ICONERROR);
	}
	pid = NULL;
	return;
}

void DropCOM() {}
