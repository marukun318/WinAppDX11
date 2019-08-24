// AppMain for Windows

#include	"AppMain.h"

extern CMD3D11		d3d11;

// コンストラクタ
AppMain::AppMain()
{
}


// デストラクタ
AppMain::~AppMain()
{
}

// 初期化
void AppMain::SetUp()
{

}

// 更新
void AppMain::Update()
{
}

// 描画
void AppMain::Draw()
{
	d3d11.Render();
}
