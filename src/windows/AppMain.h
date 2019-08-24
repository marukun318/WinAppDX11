// AppMain for Windows

#pragma once

#ifndef __AppMain_H__
#define __AppMain_H__

#include	"CMD3D11/CMD3D11.h"
#include	"WinAppDX11.h"

class AppMain
{
public:
	AppMain();
	~AppMain();

	void SetUp();
	void Update();
	void Draw();

private:
	CMD3D11Tex* offtex;
	uint32_t * offscr;
};


#endif		// __AppMain_H__
