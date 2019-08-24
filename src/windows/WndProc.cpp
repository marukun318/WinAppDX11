// WinProc.cpp : アプリケーションのエントリ ポイントを定義します。
//
// Direct3D11 基本的な初期化＋ポリゴン頂点カラー描画
//

#include	"WinAppDX11.h"
#include	<d3d11_1.h>
#include	"../windows/CMD3D11/CMD3D11.h"
#include	"../windows/CMDINPUT/CMDINPUT.h"
#include	"../windows/CMXINPUT/CMXINPUT.h"


#define MAX_LOADSTRING 100

// コメントアウトするとmmtimerを使う
#define UseQueryPerformanceCounter

// 想定フレーム数
#define FRAME_PER_SECONDS	60

// グローバル変数:
HWND                    g_hWnd = nullptr;
HINSTANCE				g_hInst = nullptr;       // 現在のインターフェイス
FLOAT					g_dpiX;
FLOAT					g_dpiY;

WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// DirectX 初期化
CMD3D11		d3d11;
CMDINPUT	dinput;		// dinput
CMXINPUT	xinput;		// xinput

//
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。
	// ref: https://msdn.microsoft.com/ja-jp/library/windows/desktop/ff485844(v=vs.85).aspx
	// ウィンドウを作成するスレッドの場合は COINIT_APARTMENTTHREADED フラグを、その他のスレッドは COINIT_MULTITHREADED を使用するのが一般的です。
	// 上記で説明したフラグに加えて、dwCoInit パラメーターに COINIT_DISABLE_OLE1DDE フラグを設定する方法も推奨されます。
	// このフラグを設定することで、現在は使用されなくなった OLE (Object Linking and Embedding) 1.0 テクノロジに伴うオーバーヘッドの一部を回避できます。
	HRESULT hr;
	if (FAILED(hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		MessageBox(nullptr, L"CoInitializeEx Init Error", L"Error", MB_OK);
		return FALSE;
	}

#ifdef _DEBUG
	// ＶＣランタイムデバッグモードＯＮ
	int nFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	nFlag &= ~(_CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF | _CRTDBG_LEAK_CHECK_DF |
		_CRTDBG_CHECK_EVERY_16_DF | _CRTDBG_CHECK_EVERY_128_DF | _CRTDBG_CHECK_EVERY_1024_DF);
	nFlag |= (_CRTDBG_ALLOC_MEM_DF | /* _CRTDBG_CHECK_CRT_DF | */ _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
	_CrtSetDbgFlag(nFlag);
#endif


#ifdef UseQueryPerformanceCounter
	LARGE_INTEGER liFreq;
	QueryPerformanceFrequency(&liFreq);
#endif

    // グローバル文字列を初期化しています。
	g_hInst = hInstance;
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINAPP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーションの初期化を実行します:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINAPP));

	// DirectX11を初期化します。
	hr = d3d11.InitDevice();
	if (FAILED(hr))
	{
		MessageBox(g_hWnd, L"DirectX11 initialization failed.", L"Error", MB_OK);
		return 0;
	}

	// DirectInput Init
	dinput.Init(hInstance, g_hWnd);
	// XInput Init
	xinput.Init();




#ifdef UseQueryPerformanceCounter
	LARGE_INTEGER liTimeOld;
	LARGE_INTEGER liTimeNow;
	LARGE_INTEGER liTimePeriod;
	QueryPerformanceCounter(&liTimeOld);
	QueryPerformanceCounter(&liTimeNow);
	liTimePeriod.QuadPart = liFreq.QuadPart / FRAME_PER_SECONDS;
#else
	timeBeginPeriod(1);
	DWORD dwTimeOld = timeGetTime();
	DWORD dwTimeNow = dwTimeOld;
#endif

	MSG msg = { 0 };

	// メイン メッセージ ループ:
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			// ここにメインタスクの処理を書く
			// 更新時間を計る
#ifdef UseQueryPerformanceCounter
			QueryPerformanceCounter(&liTimeNow);
			if ((liTimeNow.QuadPart - liTimeOld.QuadPart) >= liTimePeriod.QuadPart)
#else
			dwTimeNow = timeGetTime();
			if ((dwTimeNow - dwTimeOld) >= 17)
#endif
			{
				// 一定時間経ったなら
#ifdef UseQueryPerformanceCounter
				QueryPerformanceCounter(&liTimeOld);
#else
				dwTimeOld = dwTimeNow;
#endif
				dinput.Update();							// DirectInput 処理
				xinput.Update();							// XInput 処理
				// User job
				d3d11.Render();
			}
		}
	}

#ifndef UseQueryPerformanceCounter
	timeEndPeriod(1);
#endif

	// ライブラリの解放
	xinput.Cleanup();										// XInput
	dinput.Cleanup();										// DirectInput


	UnregisterClass(nullptr, hInstance);

    return (int) msg.wParam;
}

//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINAPP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します。
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   g_hInst = hInstance; // グローバル変数にインスタンス処理を格納します。

  //ウインドウのクライアント領域（=DirectXの描画領域）を指定
   const DWORD dwStyle = WS_OVERLAPPEDWINDOW & (~(WS_MAXIMIZEBOX |WS_THICKFRAME));
   RECT rc = { 0, 0, WIN_WIDTH, WIN_HEIGHT };
#if _MSC_VER >= 1900	// Visual Studio 2015 以降だったらWindow縦を伸ばす
   rc.bottom += 20;
#endif
   // Because the CreateWindow function takes its size in pixels,
   // obtain the system DPI and use it to scale the window size.
   ID2D1Factory1* pDirect2dFactory = nullptr;

   // Create a Direct2D factory
   D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
   options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
   HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof(ID2D1Factory1), &options, reinterpret_cast<void**>(&pDirect2dFactory));
   if (hr == E_NOINTERFACE) {
	   MessageBox(g_hWnd, L"D2D1CreateFactory() : Update required.", L"Error", MB_OK);
	   return FALSE;
   }
   if (FAILED(hr)) {
	   MessageBox(g_hWnd, L"D2D1CreateFactory() failed.", L"Error", MB_OK);
	   return FALSE;
   }

   // The factory returns the current system DPI. This is also the value it will use
   // to create its own windows.
   pDirect2dFactory->GetDesktopDpi(&g_dpiX, &g_dpiY);

   if (pDirect2dFactory != nullptr) {
	   pDirect2dFactory->Release();
	   pDirect2dFactory =  nullptr;
   }

   rc.right = static_cast<UINT>(ceil((FLOAT)rc.right * g_dpiX / 96.f));
   rc.bottom = static_cast<UINT>(ceil((FLOAT)rc.bottom * g_dpiY / 96.f));

   AdjustWindowRect(&rc, dwStyle, FALSE);

   // Window Sizeを変更させない
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, dwStyle,
	   CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);


   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hWnd = hWnd;				// グローバル変数に入れる

   return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウの描画
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

	case WM_ACTIVATE:
		if (WA_INACTIVE == wParam)
		{
			// Window非アクティブ
			dinput.Unacquire();
		}
		else {
			// Windowアクティブ
			dinput.Acquire();
		}
		return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: HDC を使用する描画コードをここに追加してください...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
