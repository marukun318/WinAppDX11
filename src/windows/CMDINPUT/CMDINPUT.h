//
// DirectInput 
// Written by Takeshi Maruyama
//

#pragma once

#ifndef __CMDINPUT_H__
#define __CMDINPUT_H__

#define DIRECTINPUT_VERSION				0x0800

#ifndef STRICT
#define STRICT						// 型チェックを厳密に行う
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN			// ヘッダーからあまり使われない関数を除く
#endif

#include	<cstdint>

#include	<windows.h>
#include	<dinput.h>
#include	<wchar.h>

#define DINPUT_PAD_MAX					4

class CMDINPUT
{
public:
	CMDINPUT();
	~CMDINPUT();

	BOOL Init(HINSTANCE inst, HWND hWnd);
	BOOL Cleanup();
	HRESULT Update();
	HRESULT GetJoyState(int);
	HRESULT GetKeyState(unsigned char *kb);

	DIJOYSTATE2 * GetState(int no);

	DWORD GetPad(int no);
	BOOL Acquire();
	BOOL Unacquire();

	inline unsigned char * GetKeyBuffer() { return key_buffer; }

private:
	BOOL bINIT = FALSE;

	LPDIRECTINPUTDEVICE8  pDIKey = nullptr;					// DirectInput キーボード

	unsigned char key_buffer[256];							// DirectInput キーボードバッファ
};

#endif	// __CMDINPUT_H__
