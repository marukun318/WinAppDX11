// Direct3D11 Render Texture View Wrapper class
// written by Takeshi Maruyam
//

#pragma once

#ifndef __CMD311Rtv_H__
#define __CMD311Rtv_H__

#include	"common.h"
#include	"CMD3D11.h"

class CMD3D11Rtv
{
public:
	CMD3D11Rtv(int, int);
	~CMD3D11Rtv();
	inline ID3D11ShaderResourceView* GetShaderResouceView() { return m_pShaderResView;  }

	ID3D11Texture2D*			m_pOffScreen = nullptr;				// オフスクリーンテクスチャ
	ID3D11ShaderResourceView*	m_pShaderResView = nullptr;			// テクスチャ シェーダーリソースビュー
	ID3D11RenderTargetView*		m_pRenderTargetView = nullptr;		// レンダーターゲットビュー
	ID3D11UnorderedAccessView*	m_pUAV = nullptr;					// テクスチャ アンオーダ－ドアクセスビュー
	int m_width;
	int m_height;

};


#endif // ! __CMD311Rtv_H__
