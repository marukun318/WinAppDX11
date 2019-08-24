// Direct3D11 Texture Wrapper class
// written by Takeshi Maruyama
//

#pragma once

#ifndef __CMD311Tex_H__
#define __CMD311Tex_H__

#include	"common.h"
#include	"CMD3D11.h"

class CMD3D11Tex
{
public:
	CMD3D11Tex(int, int);
	CMD3D11Tex(int w, int h, DXGI_FORMAT, bool, bool);

	~CMD3D11Tex();

	void Map(uint32_t *);
	void Map(uint16_t *);

	void UpdateRect(uint32_t *);
	
	void Update(uint16_t *);
	void Update(uint32_t *);

	ID3D11ShaderResourceView*  GetShaderResouceView() { return m_pShaderResView;  }

	ID3D11Texture2D*			m_pOffScreen = nullptr;				// オフスクリーンテクスチャ
	ID3D11ShaderResourceView*	m_pShaderResView = nullptr;			// テクスチャ シェーダーリソースビュー
	ID3D11UnorderedAccessView*	m_pUAV = nullptr;					// テクスチャ アンオーダ－ドアクセスビュー
	int m_width;
	int m_height;

private:
	bool m_bCpu;
	bool m_bStaging;
	void InitTexture(int w, int h, DXGI_FORMAT, bool, bool);

};


#endif // ! __CMD311Tex_H__
