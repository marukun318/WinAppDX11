// AppMain for Windows

#include	"AppMain.h"

extern CMD3D11		d3d11;

#define TEX_WIDTH	screen_width
#define TEX_HEIGHT	screen_height


// コンストラクタ
AppMain::AppMain()
{
}


// デストラクタ
AppMain::~AppMain()
{
	free(offscr);							// オフスクリーン解放
	delete offtex;
}

// 初期化
void AppMain::SetUp()
{
	offtex = new CMD3D11Tex(TEX_WIDTH, TEX_HEIGHT);	// オフスクリーンテクスチャ作成
	// オフスクリーン確保
	offscr = (uint32_t *) malloc(sizeof(uint32_t) * TEX_WIDTH * TEX_HEIGHT);
	// 白ベタテクスチャ作成
	memset(offscr, 0xFF, sizeof(uint32_t) * TEX_WIDTH * TEX_HEIGHT);
}

// 更新
void AppMain::Update()
{
	// 砂嵐
	uint32_t* ptr = offscr;
	uint32_t data = 0;
	for (int i = 0; i < TEX_HEIGHT; i++) {
		for (int j = 0; j < TEX_WIDTH; j++) {
			data = 0xFF000000 | ((rand() & 0xFF) << 16) | ((rand() & 0xFF) << 9) | (rand() & 0xFF);
//			*(ptr++) = 0xFFFF0000;		//0xAABBGGRR
			*(ptr++) = data;
		}
	}
	offtex->Update(offscr);
}

// 描画
void AppMain::Draw()
{
	d3d11.Render(offtex->m_pShaderResView);			// オフスクリーンを描画
}
