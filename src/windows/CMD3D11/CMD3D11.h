// Direct3D11
// written by maru
//
#pragma once

#ifndef __CMD3D11_H__
#define __CMD3D11_H__

#include "common.h"

#include <d3d11_1.h>
#include <d2d1_1.h>
#include <d2d1helper.h>
#include <d2d1_1helper.h>
#include <dwrite_1.h>
#include <atlbase.h>				// CComPtr の宣言を含む
#include <wrl\client.h>


#include <directxmath.h>
#include <directxcolors.h>
#include <d3dcompiler.h>

#define USE_DEPTHSTENCILVIEW		// コメントアウトすると深度/ステンシルバッファを使わない
#define USE_MSAA					// コメントアウトするとMSAA （マルチサンプル・アンチエイリアシング） を使わない

using namespace DirectX;

//
class CMD3D11
{
public:
	CMD3D11();
	~CMD3D11();

	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitDevice();
	inline ID3D11Device* GetDevice() { return m_pd3dDevice; }
	void CleanupDevice();
	void Render();
	void HandleDeviceLost();

private:
	void	ErrorDialog(const WCHAR* msg);

	HRESULT CreateDeviceResources();
	HRESULT CreateWindowSizeDependentResources();

	ID2D1Factory1*			m_pd2dFactory = nullptr;
	ID2D1Device*			m_pd2dDevice = nullptr;
	ID2D1DeviceContext*		m_pd2d1DeviceContext = nullptr;

	D3D_DRIVER_TYPE         m_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       m_featureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*           m_pd3dDevice = nullptr;
	ID3D11Device1*          m_pd3dDevice1 = nullptr;
public:
	ID3D11DeviceContext*    m_pImmediateContext = nullptr;
private:
	ID3D11DeviceContext1*   m_pImmediateContext1 = nullptr;
	IDXGISwapChain*         m_pSwapChain = nullptr;
	IDXGISwapChain1*        m_pSwapChain1 = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;
	ID3D11RasterizerState*	m_pRasterState = nullptr;
#ifdef USE_DEPTHSTENCILVIEW
	ID3D11Texture2D*		m_pDepthStencil;					// 深度/ステンシル・テクスチャを受け取る変数
	ID3D11DepthStencilView*	m_pDepthStencilView = nullptr;		// 深度/ステンシルビュー
#endif
	ID3D11VertexShader*     m_pVertexShader = nullptr;
	ID3D11PixelShader*      m_pPixelShader = nullptr;
	ID3D11InputLayout*      m_pVertexLayout = nullptr;
	ID3D11Buffer*           m_pVertexBuffer = nullptr;
	ID3D11Buffer*           m_pCBNeverChanges = nullptr;
	ID3D11Buffer*           m_pCBChangeOnResize = nullptr;
	ID3D11Buffer*           m_pCBChangesEveryFrame = nullptr;
	ID3D11Buffer*			m_pConstantBuffer = nullptr;

	ID3D11ShaderResourceView* m_pShaderResView = nullptr;		// テクスチャ
	ID3D11SamplerState*		m_pSamplerState = nullptr;			// サンプラーステート

	XMMATRIX                m_World;
	XMMATRIX                m_View;
	XMMATRIX                m_Projection;

	UINT					m_width;
	UINT					m_height;

	volatile bool			m_bLost;							// ロスト処理中
};

#endif		// __CMD3D11_H__
