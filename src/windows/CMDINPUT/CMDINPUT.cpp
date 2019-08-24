#include <windows.h>
#include "CMDINPUT.h"

#pragma comment(lib, "dinput8.lib")

#define SAFE_RELEASE(p)			{ if (p) { (p)->Release(); (p)=nullptr; } }
#define KEYDOWN(key)			(key_buffer[key] & 0x80)

//
LPDIRECTINPUT8			pDInput = nullptr;     // DirectInput
LPDIRECTINPUTDEVICE8	pDIDev[DINPUT_PAD_MAX] = { nullptr };     // DirectInput デバイス
DIDEVCAPS				diDevCaps[DINPUT_PAD_MAX] = { 0 };     // ジョイスティックの能力
DIJOYSTATE2				JoyState[DINPUT_PAD_MAX] = { 0 };

static int pad_num = 0;

static HWND _hWnd = nullptr;

// エラーメッセージ
static void ErrorMessageBox(const wchar_t* msg)
{
	MessageBox(nullptr, msg, L"Error", MB_OK);
}

// 軸モードの設定
static BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE* instance, VOID* context)
{
	HWND hDlg = (HWND)context;

	DIPROPRANGE propRange;
	propRange.diph.dwSize = sizeof(DIPROPRANGE);
	propRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	propRange.diph.dwHow = DIPH_BYID;
	propRange.diph.dwObj = instance->dwType;
	propRange.lMin = -128;
	propRange.lMax = 127;

	LPDIRECTINPUTDEVICE8 pDev = pDIDev[pad_num];

	// Set the range for the axis
	if (FAILED(pDev->SetProperty(DIPROP_RANGE, &propRange.diph))) {
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

// ジョイスティックを列挙する関数
static BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	HRESULT hr;

	// 列挙されたジョイスティックへのインターフェイスを取得する。
	hr = pDInput->CreateDevice(pdidInstance->guidInstance, &pDIDev[pad_num], nullptr);
	if (FAILED(hr))
		return DIENUM_CONTINUE;

	// ジョイスティックの能力を調べる
	memset(&diDevCaps[pad_num], 0, sizeof(DIDEVCAPS));
	diDevCaps[pad_num].dwSize = sizeof(DIDEVCAPS);
	hr = pDIDev[pad_num]->GetCapabilities(&diDevCaps[pad_num]);
	if (FAILED(hr)) {
		// ジョイスティック能力の取得に失敗
		SAFE_RELEASE(pDIDev[pad_num]);
		return DIENUM_CONTINUE;
	}

	// データ形式を設定
	hr = pDIDev[pad_num]->SetDataFormat(&c_dfDIJoystick2);
	if (FAILED(hr)) {
		//		SysPrintf(L"c_dfDIJoystick2形式の設定に失敗\n");
		SAFE_RELEASE(pDIDev[pad_num]);
		return DIENUM_CONTINUE;
	}

	// 協調モードを設定（フォアグラウンド＆非排他モード）
	hr = pDIDev[pad_num]->SetCooperativeLevel(_hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr)) {
		//		SysPrintf(L"フォアグラウンド＆非排他モードの設定に失敗\n");
		SAFE_RELEASE(pDIDev[pad_num]);
		return DIENUM_CONTINUE;
	}

	// コールバック関数を使って各軸のモードを設定
	hr = pDIDev[pad_num]->EnumObjects(EnumAxesCallback, NULL, DIDFT_AXIS);
	if (FAILED(hr)) {
		//		SysPrintf(L"軸モードの設定に失敗\n");
		SAFE_RELEASE(pDIDev[pad_num]);
		return DIENUM_CONTINUE;
	}

	// 入力制御開始
	pDIDev[pad_num]->Acquire();

	pad_num++;

	return (pad_num < DINPUT_PAD_MAX) ? DIENUM_CONTINUE : DIENUM_STOP;
}

// コンストラクタ
CMDINPUT::CMDINPUT()
{
}

// デストラクタ
CMDINPUT::~CMDINPUT()
{
}

// 初期化
BOOL CMDINPUT::Init(HINSTANCE inst, HWND hWnd) {
	HRESULT hr = 0;

	_hWnd = hWnd;

	// DirectInputの作成
	hr = DirectInput8Create(inst, DIRECTINPUT_VERSION,
		IID_IDirectInput8, (void**)&pDInput, NULL);
	if (FAILED(hr)) {
		ErrorMessageBox(L"DirectInput8オブジェクトの作成に失敗");
		return FALSE;
	}

	// デバイスを列挙して作成
	pad_num = 0;
	hr = pDInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback,
		NULL, DIEDFL_ATTACHEDONLY);
	if (FAILED(hr)) {
		ErrorMessageBox(L"DirectInputDevice8デバイスの列挙に失敗");
		return hr;
	}

	//	SysPrintf(L"DirectInputDevice8=%d\n", pad_num);

		// キーボードデバイスの作成
	hr = pDInput->CreateDevice(GUID_SysKeyboard, &pDIKey, nullptr);
	if FAILED(hr) {
		ErrorMessageBox(L"Keyboard Device Failure");
		return FALSE;
	}
	hr = pDIKey->SetDataFormat(&c_dfDIKeyboard);
	if FAILED(hr) {
		ErrorMessageBox(L"Keyboard Device Failure");
		return FALSE;
	}

	// Set the cooperative level 
	hr = pDIKey->SetCooperativeLevel(_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if FAILED(hr) {
		ErrorMessageBox(L"Keyboard Device Failure");
		return FALSE;
	}

	if (pDIKey) pDIKey->Acquire();

	bINIT = TRUE;

	return TRUE;
}

// 
BOOL CMDINPUT::Acquire() {

	if (!bINIT)
		return FALSE;

	for (int i = 0; i < DINPUT_PAD_MAX; i++) {
		if (pDIDev[i]) {
			pDIDev[i]->Acquire();
		}
	}

	if (pDIKey) pDIKey->Acquire();

	return TRUE;
}

//
BOOL CMDINPUT::Unacquire() {
	if (!bINIT)
		return FALSE;

	for (int i = 0; i < DINPUT_PAD_MAX; i++) {
		if (pDIDev[i]) {
			pDIDev[i]->Unacquire();
		}
	}

	if (pDIKey) pDIKey->Unacquire();

	return TRUE;
}

// 解放
BOOL CMDINPUT::Cleanup() {

	for (int i = DINPUT_PAD_MAX - 1; i >= 0; i--) {
		if (pDIDev[i]) {
			pDIDev[i]->Unacquire();
			pDIDev[i]->Release();
			pDIDev[i] = nullptr;
		}
	}

	if (pDIKey) {
		pDIKey->Unacquire();
		pDIKey->Release();
		pDIKey = nullptr;
	}

	SAFE_RELEASE(pDInput);

	if (!bINIT)
		return FALSE;

	return TRUE;
}


// 更新
HRESULT CMDINPUT::Update() {
	HRESULT hr;

	if (!bINIT)
		return E_FAIL;

	// Gamepad Update
	for (int i = 0; i < DINPUT_PAD_MAX; i++) {
		if (pDIDev[i]) {
			hr = pDIDev[i]->Poll();
			if (FAILED(hr)) {
				// DInput is telling us that the input stream has been
				// interrupted. We aren't tracking any state between polls, so
				// we don't have any special reset that needs to be done. We
				// just re-acquire and try again.
				hr = pDIDev[i]->Acquire();
				while (hr == DIERR_INPUTLOST) {
					hr = pDIDev[i]->Acquire();
				}

				// If we encounter a fatal error, return failure.
				if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED)) {
					return E_FAIL;
				}

				// If another application has control of this device, return successfully.
				// We'll just have to wait our turn to use the joystick.
				if (hr == DIERR_OTHERAPPHASPRIO) {
					return S_OK;
				}
			}

			// Get the input's device state
			if (FAILED(hr = pDIDev[i]->GetDeviceState(sizeof(DIJOYSTATE2), &JoyState[i]))) {
				return hr; // The device should have been acquired during the Poll()
			}

		}
	}

	// Keyboard Update
	hr = pDIKey->GetDeviceState(sizeof(key_buffer), (LPVOID)&key_buffer);
	if (FAILED(hr)) {
		if (hr == DIERR_INPUTLOST) {
			pDIKey->Acquire();
		}
		return E_FAIL;
	}


	return S_OK;
}

// キーボードだけ更新
HRESULT CMDINPUT::GetKeyState(unsigned char *kb)
{
	HRESULT hr;

	if (!bINIT)
		return E_FAIL;

	// Keyboard Update
	hr = pDIKey->GetDeviceState(sizeof(key_buffer), (LPVOID)kb);
	if (FAILED(hr)) {
		if (hr == DIERR_INPUTLOST) {
			pDIKey->Acquire();
		}
		return E_FAIL;
	}

	return S_OK;
}


// ゲームパッドだけ更新
HRESULT CMDINPUT::GetJoyState(int i)
{
	HRESULT hr;

	if (!bINIT)
		return E_FAIL;

	// Gamepad Update
	if (pDIDev[i]) {
		hr = pDIDev[i]->Poll();
		if (FAILED(hr)) {
			// DInput is telling us that the input stream has been
			// interrupted. We aren't tracking any state between polls, so
			// we don't have any special reset that needs to be done. We
			// just re-acquire and try again.
			hr = pDIDev[i]->Acquire();
			while (hr == DIERR_INPUTLOST) {
				hr = pDIDev[i]->Acquire();
			}

			// If we encounter a fatal error, return failure.
			if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED)) {
				return E_FAIL;
			}

			// If another application has control of this device, return successfully.
			// We'll just have to wait our turn to use the joystick.
			if (hr == DIERR_OTHERAPPHASPRIO) {
				return S_OK;
			}
		}

		// Get the input's device state
		if (FAILED(hr = pDIDev[i]->GetDeviceState(sizeof(DIJOYSTATE2), &JoyState[i]))) {
			return hr; // The device should have been acquired during the Poll()
		}

	}

	return S_OK;
}


// 取得
DIJOYSTATE2* CMDINPUT::GetState(int no)
{
	if (no < 0 || no >= DINPUT_PAD_MAX)
		return nullptr;

	return &JoyState[no];
}

// 取得
DWORD CMDINPUT::GetPad(int no)
{
	DWORD pad = 0;

	// GamePad 状態取得
	DIJOYSTATE2 * js2 = GetState(no);
	if (js2 != nullptr)
	{
		//		SysPrintf(L"lX=%ld lY=%ld ", js2->lX, js2->lY);
		for (int j = 0; j < 128; j++) {
			if (js2->rgbButtons[j] & 0x80) {
				//				SysPrintf(L"[%d] ", j);
			}

		}
		//		SysPrintf(L"\n");
	}

#if 0
	// Keyboard 状態取得
	bool keyflag = false;
	if (KEYDOWN(DIK_UP)) {
		SysPrintf(L"UP ");
		keyflag = true;
	}
	if (KEYDOWN(DIK_DOWN)) {
		SysPrintf(L"DOWN ");
		keyflag = true;
	}
	if (KEYDOWN(DIK_LEFT)) {
		SysPrintf(L"LEFT ");
		keyflag = true;
	}
	if (KEYDOWN(DIK_RIGHT)) {
		SysPrintf(L"RIGHT ");
		keyflag = true;
	}
	if (keyflag) {
		SysPrintf(L"\n");
	}
#endif

	return pad;
}
