#pragma once

#ifndef __D3D11_COMMON_H__
#define __D3D11_COMMON_H__

#include	"../targetver.h"

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
#include	<d3d11_1.h>
#include	<atlbase.h>				// CComPtr の宣言を含む
#include	<wrl\client.h>

#endif // ! __D3D11_COMMON_H__

