#include "CMD3D11.h"

#define SAFE_RELEASE(x) if( (x) != nullptr ){ (x)->Release(); x = nullptr; }

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
extern HINSTANCE        g_hInst;
extern HWND             g_hWnd;
extern FLOAT			g_dpiX;
extern FLOAT			g_dpiY;

//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------
struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
	XMFLOAT2 UV;
};

struct CBNeverChanges
{
	XMMATRIX mView;
};

struct CBChangeOnResize
{
	XMMATRIX mProjection;
};

struct CBChangesEveryFrame
{
	XMMATRIX mWorld;
};

//--------------------------------------------------------------------------------------
// Vertices
//--------------------------------------------------------------------------------------
static SimpleVertex const vertices[] =
{
	// 2D Vertex
	{ XMFLOAT3(0.0f,0.0f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(640.0f,0.0f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(0.0f, 400.0f, 0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(640.0f, 400.0f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
};

//--------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------
CMD3D11::CMD3D11()
{
	m_bLost = false;
}

//--------------------------------------------------------------------------------------
// デストラクタ
//--------------------------------------------------------------------------------------
CMD3D11::~CMD3D11()
{
	CleanupDevice();
}

//--------------------------------------------------------------------------------------
// Error Dialog
//--------------------------------------------------------------------------------------
void CMD3D11::ErrorDialog(const WCHAR* msg)
{
	MessageBox(nullptr, msg, L"Error", MB_OK);
}

//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CMD3D11::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT CMD3D11::InitDevice()
{
	HRESULT hr;

	hr = CreateDeviceResources();
	if (FAILED(hr)) {
		return hr;
	}

	hr = CreateWindowSizeDependentResources();
	if (FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create Device Resources
//--------------------------------------------------------------------------------------
HRESULT CMD3D11::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	m_width = rc.right - rc.left;
	m_height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE const driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL const featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	const UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = m_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
#if 0
			// Create a Direct2D factory
			D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
			options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
			hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1), &options, reinterpret_cast<void**>(&m_pd2dFactory));
			if (hr == E_NOINTERFACE) {
				ErrorDialog(L"D2D1CreateFactory() : Update needed.");
				return hr;
			}
			if (FAILED(hr)) {
				ErrorDialog(L"D2D1CreateFactory() failed.");
				return hr;
			}

			//
			// Direct2Dデバイスの作成
			//
			hr = m_pd2dFactory->CreateDevice(dxgiDevice, &m_pd2dDevice);
			if (FAILED(hr)) {
				ErrorDialog(L"Direct2Dデバイスの作成󠄁に失敗しました。");
				//		dxgiDevice->Release();
				return hr;
			}
			// Direct2Dデバイスコンテキストの作成
			hr = m_pd2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pd2d1DeviceContext);
			if (FAILED(hr)) {
				ErrorDialog(L"Direct2Dデバイスコンテキストの作成󠄁に失敗しました。");
				return hr;
			}
#endif
			//
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}

	// 現環境で使用できるMSAAをチェック
	// 現状では最大スペックのMSAAを使う
#ifdef USE_MSAA
	DXGI_SAMPLE_DESC sampleDesc = { 0 };
	for (int i = 1; i <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; i <<= 1)
	{
		UINT Quality;
		if (SUCCEEDED(m_pd3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_D32_FLOAT/*DXGI_FORMAT_D24_UNORM_S8_UINT*/, i, &Quality)))
		{
			if (0 < Quality)
			{
				sampleDesc.Count = i;
				sampleDesc.Quality = Quality - 1;
			}
		}
	}
//	Printf(L"sampleDesc.Count = %d\n", sampleDesc.Count);
//	Printf(L"sampleDesc.Quality = %d\n", sampleDesc.Quality);
#endif

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = m_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&m_pd3dDevice1));
		if (SUCCEEDED(hr))
		{
			(void)m_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&m_pImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = m_width;
		sd.Height = m_height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
#ifndef USE_MSAA
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
#else
		sd.SampleDesc = sampleDesc;
#endif
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &m_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = m_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&m_pSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = m_width;
		sd.BufferDesc.Height = m_height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
//		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;		// これをつけるとフルスクリーン時の解像度が実解像度になる
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = g_hWnd;
#ifndef USE_MSAA
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
#else
		sd.SampleDesc = sampleDesc;
#endif
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(m_pd3dDevice, &sd, &m_pSwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
//	dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

#ifdef	USE_DEPTHSTENCILVIEW
	// デプスステンシルビュー(DSV) を 作成
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = m_width;		// 幅
	descDepth.Height = m_height;	// 高さ
	descDepth.MipLevels = 1;		// ミップマップ レベル数
	descDepth.ArraySize = 1;		// 配列サイズ
	descDepth.Format = DXGI_FORMAT_D32_FLOAT; // フォーマット(深度のみ)
											  //	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
#ifndef USE_MSAA
	descDepth.SampleDesc.Count = 1;    // マルチサンプリングの設定
	descDepth.SampleDesc.Quality = 0;    // マルチサンプリングの品質
#else
	descDepth.SampleDesc = sampleDesc;
#endif
	descDepth.Usage = D3D11_USAGE_DEFAULT;		// 使用方法 デフォルト
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;	// 深度/ステンシルとして使用
	descDepth.CPUAccessFlags = 0;    // CPUからアクセスしない
	descDepth.MiscFlags = 0;    // その他の設定なし
	hr = m_pd3dDevice->CreateTexture2D(
		&descDepth,				// 作成する2Dテクスチャ
		nullptr,				// 
		&m_pDepthStencil	// 作成したテクスチャを受け取る
	);
	if (FAILED(hr))
		return hr;

#ifdef USE_MSAA
	// 深度/ステンシルビューの作成 (MSAAあり)
	D3D11_DEPTH_STENCIL_VIEW_DESC descDsv = {};
	descDsv.Format = descDepth.Format;
	descDsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	descDsv.Flags = 0;
	descDsv.Texture2D.MipSlice = 0;
	m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDsv, &m_pDepthStencilView);
#else
	// 深度/ステンシルビューの作成 (MSAAなし)
	g_pd3dDevice->CreateDepthStencilView(g_pDepthStencil, nullptr, &g_pDepthStencilView);
#endif // USE_MSAA

#endif // USE_DEPTHSTENCILVIEW

#ifdef USE_DEPTHSTENCILVIEW
	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
#else	
	g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);
#endif

	// RasterState
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;		// D3D11_CULL_FRONT or D3D11_CULL_BACK or D3D11_CULL_NONE
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = false;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = true;
#ifdef USE_MSAA
	rasterDesc.MultisampleEnable = true;
#else
	rasterDesc.MultisampleEnable = false;
#endif
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	hr = m_pd3dDevice->CreateRasterizerState(&rasterDesc, &m_pRasterState);
	if (FAILED(hr)) return hr;
	m_pImmediateContext->RSSetState(m_pRasterState);

	return hr;
}

//--------------------------------------------------------------------------------------
//  CreateWindowSizeDependentResources
//--------------------------------------------------------------------------------------
HRESULT CMD3D11::CreateWindowSizeDependentResources()
{
	HRESULT hr = S_OK;
	const char* VS_ver = "vs_5_0";
	const char* PS_ver = "ps_5_0";

	// Create the constant buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBNeverChanges);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pCBNeverChanges);
	if (FAILED(hr))
		return hr;

	bd.ByteWidth = sizeof(CBChangeOnResize);
	hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pCBChangeOnResize);
	if (FAILED(hr))
		return hr;

	bd.ByteWidth = sizeof(CBChangesEveryFrame);
	hr = m_pd3dDevice->CreateBuffer(&bd, nullptr, &m_pCBChangesEveryFrame);
	if (FAILED(hr))
		return hr;

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)m_width;
	vp.Height = (FLOAT)m_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pImmediateContext->RSSetViewports(1, &vp);

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	do {
		hr = CompileShaderFromFile(L"shader.hlsl", "VS", VS_ver, &pVSBlob);
		if (FAILED(hr))
			break;

		// Create the vertex shader
		hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);
		if (FAILED(hr)) {
			SAFE_RELEASE(pVSBlob);
			break;
		}

		// Compile the pixel shader
		ID3DBlob* pPSBlob = nullptr;
		hr = CompileShaderFromFile(L"shader.hlsl", "PS", PS_ver, &pPSBlob);
		if (FAILED(hr))
			break;

		// Create the pixel shader
		hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader);
		SAFE_RELEASE(pPSBlob);
		if (FAILED(hr))
			break;
	} while (0);

	if (FAILED(hr)) {
		ErrorDialog(L"The HLSL file cannot be compiled.  Please run this executable from the directory that contains the HLSL file.");
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC const layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &m_pVertexLayout);

	SAFE_RELEASE(pVSBlob);

	if (FAILED(hr)) {
		ErrorDialog(L"CreateInputLayout() Failed.");
		return hr;
	}

	// Set the input layout
	m_pImmediateContext->IASetInputLayout(m_pVertexLayout);

	// サンプラーステートの設定
	// D3D11_SAMPLER_DESC
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;         // サンプリング時に使用するフィルタ。ここでは異方性フィルターを使用する。
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある u テクスチャー座標の描画方法
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある v テクスチャー座標
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;     // 0 ～ 1 の範囲外にある w テクスチャー座標
	samplerDesc.MipLODBias = 0;                            // 計算されたミップマップ レベルからのバイアス
	samplerDesc.MaxAnisotropy = 16;                        // サンプリングに異方性補間を使用している場合の限界値。有効な値は 1 ～ 16 。
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;  // 比較オプション。

//	samplerDesc.BorderColor = XMVECTOR4(0.0f, 0.0f, 0.0f, 0.0f); // 境界色
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;

	samplerDesc.MinLOD = 0;                                // アクセス可能なミップマップの下限値
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;                // アクセス可能なミップマップの上限値
   // ID3D11Device::CreateSamplerState
	hr = m_pd3dDevice->CreateSamplerState(&samplerDesc, &m_pSamplerState);

	// Create vertex buffer
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	hr = m_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (FAILED(hr)) {
		ErrorDialog(L"CreateVertexBuffer Failed.");
		return hr;
	}

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Set primitive topology
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Initialize the world matrices
	m_World = XMMatrixIdentity();

	// Initialize the view matrix
	m_View = XMMatrixIdentity();

	CBNeverChanges cbNeverChanges;
	cbNeverChanges.mView = XMMatrixTranspose(m_View);
	m_pImmediateContext->UpdateSubresource(m_pCBNeverChanges, 0, nullptr, &cbNeverChanges, 0, 0);

	// プロジェクショントランスフォーム
	//	g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (FLOAT)height, 0.1f, 100.0f);
	m_Projection = XMMatrixOrthographicOffCenterLH(0.0f, (FLOAT)m_width, (FLOAT)m_height, 0.0f, 0.0f, 100.0f);

	CBChangeOnResize cbChangesOnResize;
	cbChangesOnResize.mProjection = XMMatrixTranspose(m_Projection);
	m_pImmediateContext->UpdateSubresource(m_pCBChangeOnResize, 0, nullptr, &cbChangesOnResize, 0, 0);

	return hr;
}

//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CMD3D11::CleanupDevice()
{
	if (m_pImmediateContext) m_pImmediateContext->ClearState();

	SAFE_RELEASE(m_pSamplerState);
	SAFE_RELEASE(m_pShaderResView);

	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pVertexLayout);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);

	SAFE_RELEASE(m_pConstantBuffer);
	SAFE_RELEASE(m_pCBNeverChanges);
	SAFE_RELEASE(m_pCBChangeOnResize);
	SAFE_RELEASE(m_pCBChangesEveryFrame);

#ifdef USE_DEPTHSTENCILVIEW
	SAFE_RELEASE(m_pDepthStencilView);
	SAFE_RELEASE(m_pDepthStencil);
#endif

	SAFE_RELEASE(m_pRasterState);
	SAFE_RELEASE(m_pRenderTargetView);
	SAFE_RELEASE(m_pSwapChain1);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pImmediateContext1);
	SAFE_RELEASE(m_pImmediateContext);
	SAFE_RELEASE(m_pd3dDevice1);
	SAFE_RELEASE(m_pd3dDevice);

	SAFE_RELEASE(m_pd2dFactory);
	SAFE_RELEASE(m_pd2dDevice);
	SAFE_RELEASE(m_pd2d1DeviceContext);
}

//--------------------------------------------------------------------------------------
// Render the frame
//--------------------------------------------------------------------------------------
void CMD3D11::Render()
{
	Render(m_pShaderResView);
}

//
void CMD3D11::Render(ID3D11ShaderResourceView* shaderResView)
{
	if (m_bLost) return;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	// Just clear the backbuffer
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, Colors::MidnightBlue);

#ifdef USE_DEPTHSTENCILVIEW
	// 深度/ステンシル値のクリア
	m_pImmediateContext->ClearDepthStencilView(
		m_pDepthStencilView,	// クリアする深度/ステンシルビュー
		D3D11_CLEAR_DEPTH,	// 深度値だけクリアする
		1.0f,			// 深度バッファをクリアする値
		0			// ステンシルバッファをクリアする値(今回は関係なし)
	);
#endif
	// Update variables that change once per frame
	//
	{
		CBChangesEveryFrame cb;
		cb.mWorld = XMMatrixTranspose(m_World);
		//	cb.vMeshColor = g_vMeshColor;
		m_pImmediateContext->UpdateSubresource(m_pCBChangesEveryFrame, 0, nullptr, &cb, 0, 0);
	}

	//このコンスタントバッファーをどのシェーダーで使うか
	// Set Vertex shader
	m_pImmediateContext->VSSetShader(m_pVertexShader, nullptr, 0);
	m_pImmediateContext->VSSetConstantBuffers(1, 1, &m_pCBNeverChanges);
	m_pImmediateContext->VSSetConstantBuffers(2, 1, &m_pCBChangeOnResize);
	m_pImmediateContext->VSSetConstantBuffers(3, 1, &m_pCBChangesEveryFrame);

	// Set Pixel shader
	m_pImmediateContext->PSSetShader(m_pPixelShader, nullptr, 0);
	m_pImmediateContext->PSSetConstantBuffers(3, 1, &m_pCBChangesEveryFrame);

	// Set the input layout
	m_pImmediateContext->IASetInputLayout(m_pVertexLayout);

	// Set primitive topology
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// テクスチャとサンプラ設定
	m_pImmediateContext->PSSetShaderResources(0, 1, &shaderResView);
	m_pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerState);

	// Render a quad
	m_pImmediateContext->Draw(4, 0);

	// Present the information rendered to the back buffer to the front buffer (the screen)
	HRESULT hr = m_pSwapChain->Present(0, 0);
	// If the device was removed either by a disconnection or a driver upgrade, we
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
		//
		HandleDeviceLost();
	}
}

//--------------------------------------------------------------------------------------
// Device lost process
//--------------------------------------------------------------------------------------
void CMD3D11::HandleDeviceLost()
{
	HRESULT reason = m_pd3dDevice->GetDeviceRemovedReason();

	m_bLost = true;

#ifdef _DEBUG
	WCHAR outString[100];
	size_t size = 100;
	swprintf_s(outString, size, L"Device removed! DXGI_ERROR code: 0x%X\n", reason);
	OutputDebugString(outString);
	ErrorDialog(outString);
#endif

	CleanupDevice();

	HRESULT hr = CreateDeviceResources();
	if (FAILED(hr)) {
#ifdef _DEBUG
		swprintf_s(outString, size, L"CreateDeviceResources() ERROR code: 0x%X\n", reason);
		OutputDebugString(outString);
		ErrorDialog(outString);
#endif
	}

//	m_pd2d1DeviceContext->SetDpi(g_dpiX, g_dpiY);

	hr = CreateWindowSizeDependentResources();
	if (FAILED(hr)) {
#ifdef _DEBUG
		swprintf_s(outString, size, L"CreateWindowSizeDependentResources() ERROR code: 0x%X\n", reason);
		OutputDebugString(outString);
		ErrorDialog(outString);
#endif
	}


	m_bLost = false;
}
