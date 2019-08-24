// XInput Written by Takeshi Maruyama

#pragma once

#ifndef __CM_XINPUT_H__
#define __CM_XINPUT_H__


#include <windows.h>
#include <Xinput.h>

// DEFINE
#define XI_MAX_CONTROLLERS  4						// XInputが認識できるのは4つまで
#define	XI_Threshold		65535/4					// しきい値

#define XPAD_RAW_LEFT		0x00000001				// LEFT
#define XPAD_RAW_RIGHT		0x00000002				// RIGHT
#define XPAD_RAW_UP			0x00000004				// UP
#define XPAD_RAW_DOWN		0x00000008				// DOWN

#define XPAD_RAW_A			0x00000010				// A button
#define XPAD_RAW_B			0x00000020				// B button
#define XPAD_RAW_X			0x00000040				// X button
#define XPAD_RAW_Y			0x00000080				// Y button
#define XPAD_RAW_L1			0x00000100				// L button
#define XPAD_RAW_R1			0x00000200				// R button
#define XPAD_RAW_L2			0x00000400				// L button
#define XPAD_RAW_R2			0x00000800				// R button
#define XPAD_RAW_BACK		0x00001000				// BACK button
#define XPAD_RAW_START		0x00002000				// START button

//
typedef struct _CONTROLER_STATE
{
	XINPUT_STATE state;
	bool bConnected;
} CONTROLER_STATE, *pCONTROLER_STATE;

//
class CMXINPUT
{
public:
	CMXINPUT();
	~CMXINPUT();

	// Forward
	bool		Init();
	bool		Cleanup();
	bool		Update();

	bool		Is_GamePad_Connected(int);
	DWORD		Get_GamePad_RAW(int);

private:
	HRESULT		UpdateControllerState();

	// ワークエリア
	CONTROLER_STATE GAME_PAD[XI_MAX_CONTROLLERS];

};


#endif // __CM_XINPUT_H__
