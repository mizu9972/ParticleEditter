//*****************************************************************************
//!	@file	game.cpp
//!	@brief	
//!	@note	ゲーム処理
//!	@author	
//*****************************************************************************

//-----------------------------------------------------------------------------
//	Include header files.
//-----------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include <Crtdbg.h>
#include <DirectXMath.h>
#include <random>
#include "DX11util.h"
#include "CModel.h"
#include "game.h"
#include "CCamera.h"
#include "CDirectInput.h"
#include "DX11Settransform.h"
#include "dx11mathutil.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#include "CXFileParticleSystem.h"
//パーティクルで使用するテクスチャの名前
constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/DefaultParticle.png";
//-----------------------------------------------------------------------------
// グローバル変数
//-----------------------------------------------------------------------------

CModel				g_skydome;
XMFLOAT4X4			g_matskydome;

ParticleSystem g_NomalParticleSystem;
XFileParticleSystem g_XfileParticleSystem;
t_ParticleSystemState g_setParticleState;
t_ParticleSystemState g_ParticleState4ImGui;

extern int g_nCountFPS;//FPS

//-----------------------------------------------------------------------------
// プロトタイプ宣言
//-----------------------------------------------------------------------------
void ImGuiDraw();
bool isPartilceStateEqual();
//==============================================================================
//!	@fn		GameInit
//!	@brief	ゲーム初期処理
//!	@param	インスタンス値
//!	@param	ウインドウハンドル値
//!	@param	ウインドウサイズ幅（補正済みの値）
//!	@param	ウインドウサイズ高さ（補正済みの値）
//!	@param　	フルスクリーンフラグ　true ; フルスクリーン　false : ウインドウ
//!	@retval	true 成功　false 失敗
//==============================================================================
bool GameInit(HINSTANCE hinst, HWND hwnd, int width, int height, bool fullscreen)
{
	bool		sts;

	// DX11初期処理
	sts = DX11Init(hwnd,width,height,fullscreen);
	if (!sts) {
		MessageBox(hwnd, "DX11 init error", "error", MB_OK);
		return false;
	}

	// im gui 初期化
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(GetDX11Device(), GetDX11DeviceContext());

	ImGui::StyleColorsClassic();

	// DIRECTINPUT初期化
	CDirectInput::GetInstance().Init(hinst, hwnd, width, height);

	//パーティクルシステム設定--------------------
	const char* filename_(PARTICLE_TEXTURE);

	g_setParticleState.m_PositionX = 0;
	g_setParticleState.m_PositionY = 0;
	g_setParticleState.m_PositionZ = 0;

	g_setParticleState.m_AngleX = 0;
	g_setParticleState.m_AngleY = 0;
	g_setParticleState.m_AngleZ = 0;

	g_setParticleState.m_AngleRangeX = 360;
	g_setParticleState.m_AngleRangeY = 360;
	g_setParticleState.m_AngleRangeZ = 360;

	g_setParticleState.m_DuaringTime = 10;

	g_setParticleState.m_MaxLifeTime = 5;
	g_setParticleState.m_ParticleNum = 100;
	g_setParticleState.m_Size = 20;
	g_setParticleState.m_Speed = 30;

	XMFLOAT4 setColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	memcpy(&g_setParticleState.m_Color, &setColor, sizeof(XMFLOAT4));
	g_setParticleState.isLooping = true;
	//----------------------------------------

	//ImGui用に保存
	memcpy(&g_ParticleState4ImGui, &g_setParticleState, sizeof(t_ParticleSystemState));

	//g_XfileParticleSystem.ModelInit("assets/f1.x.dat", "shader/vs.fx", "shader/ps.fx");
	//g_XfileParticleSystem.Init(g_setParticleState);

	g_NomalParticleSystem.Init(g_setParticleState, filename_);
	// スカイドーム読み込み
	g_skydome.Init("assets/skydome.x.dat","shader/vs.fx","shader/psskydome.fx");

	DX11MtxIdentity(g_matskydome);


	// カメラ変換行列初期化
	// プロジェクション変換行列初期化
	XMFLOAT3 eye =	  {0,0,-50};				// 視点
	XMFLOAT3 lookat = { 0,0,0 };			// 注視点
	XMFLOAT3 up = {0,1,0};					// 上向きベクトル

	CCamera::GetInstance()->Init(1.0f, 10000.0f, XM_PIDIV2, SCREEN_X, SCREEN_Y, eye, lookat, up);

	// 平行光源初期化
	DX11LightInit(DirectX::XMFLOAT4(1,1,-1,0));		// 平行光源の方向をセット

	return	true;
}

//==============================================================================
//!	@fn		GameInput
//!	@brief	ゲーム入力検知処理
//!	@param　	
//!	@retval	なし
//==============================================================================
void GameInput(){
	CDirectInput::GetInstance().GetKeyBuffer();			// キー入力取得
}

//==============================================================================
//!	@fn		GameUpdate
//!	@brief	ゲーム更新処理
//!	@param　	
//!	@retval	なし
//==============================================================================
void GameUpdate(){
	g_NomalParticleSystem.Update();
}

//==============================================================================
//!	@fn		GameRender
//!	@brief	ゲーム更新処理
//!	@param　	
//!	@retval	なし
//==============================================================================
void GameRender() {
	// ターゲットバッファクリア	
	float ClearColor[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; //red,green,blue,alpha

	DirectX::XMFLOAT4X4 mat;

	// レンダリング前処理
	DX11BeforeRender(ClearColor);

	// ビュー変換行列セット
	mat = CCamera::GetInstance()->GetCameraMatrix();
	DX11SetTransform::GetInstance()->SetTransform(DX11SetTransform::TYPE::VIEW, mat);

	// プロジェクション変換行列セット
	mat = CCamera::GetInstance()->GetProjectionMatrix();
	DX11SetTransform::GetInstance()->SetTransform(DX11SetTransform::TYPE::PROJECTION, mat);

	// ワールド変換行列
	DX11SetTransform::GetInstance()->SetTransform(DX11SetTransform::TYPE::WORLD, g_matskydome);
	g_skydome.Draw();

	CDirectXGraphics::GetInstance()->TurnOnAlphaBlending();//アルファブレンドを有効に
	g_NomalParticleSystem.Draw();

	bool show_demo_window = true;
	bool show_another_window = false;

	ImGuiDraw();
	// レンダリング後処理
	DX11AfterRender();
}

//==============================================================================
//!	@fn		GameMain
//!	@brief	ゲームループ処理
//!	@param　	
//!	@retval	なし
//==============================================================================
void GameMain()
{
	GameInput();					// 入力
	GameUpdate();					// 更新
	GameRender();					// 描画
}

//==============================================================================
//!	@fn		GameExit
//!	@brief	ゲーム終了処理
//!	@param　	
//!	@retval	なし
//==============================================================================
void GameExit()
{
	g_NomalParticleSystem.UnInit();
	DX11Uninit();
}

//==============================================================================
//!	@fn		ImGuiDraw
//!	@brief	ゲーム終了処理
//!	@param　	
//!	@retval	なし
//==============================================================================
void ImGuiDraw() {
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	static int counter = 0;

	const char* filename_(PARTICLE_TEXTURE);
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).

	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.0f, 0.7f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.0f, 0.3f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 300), ImGuiSetCond_Once);

	ImGui::Begin("ParticleSystem Settings");
	std::string str;

	ImGui::Text("FPS:%d", g_nCountFPS);

	if (ImGui::TreeNode("Position")) {
		ImGui::SliderFloat("X", &g_ParticleState4ImGui.m_PositionX, -1000.0f, 1000.0f);
		ImGui::SliderFloat("Y", &g_ParticleState4ImGui.m_PositionY, -1000.0f, 1000.0f);
		ImGui::SliderFloat("Z", &g_ParticleState4ImGui.m_PositionZ, -1000.0f, 1000.0f);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Angle")) {
		ImGui::SliderInt("X", &g_ParticleState4ImGui.m_AngleX, 1, 360);
		ImGui::SliderInt("Y", &g_ParticleState4ImGui.m_AngleY, 1, 360);
		ImGui::SliderInt("Z", &g_ParticleState4ImGui.m_AngleZ, 1, 360);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("AngleRange")) {
		ImGui::SliderInt("X", &g_ParticleState4ImGui.m_AngleRangeX, 1, 360);
		ImGui::SliderInt("Y", &g_ParticleState4ImGui.m_AngleRangeY, 1, 360);
		ImGui::SliderInt("Z", &g_ParticleState4ImGui.m_AngleRangeZ, 1, 360);
		ImGui::TreePop();

	}
	ImGui::SliderFloat("Duaring", &g_ParticleState4ImGui.m_DuaringTime, 0, 1000);


	ImGui::SliderInt("ParticleNum", &g_ParticleState4ImGui.m_ParticleNum, 0, 1000);
	ImGui::SliderFloat("MaxLifeTime", &g_ParticleState4ImGui.m_MaxLifeTime, 0.0f, 1000.0f);
	ImGui::SliderFloat("Size", &g_ParticleState4ImGui.m_Size, 0.0f, 1000.0f);
	ImGui::SliderFloat("Speed", &g_ParticleState4ImGui.m_Speed, 0.0f, 1000.0f);
	ImGui::Checkbox("isLooping", &g_ParticleState4ImGui.isLooping);


	ImGui::ColorEdit4("Color", g_ParticleState4ImGui.m_Color, 0);

	if (isPartilceStateEqual() == false) {
		memcpy(&g_setParticleState, &g_ParticleState4ImGui, sizeof(t_ParticleSystemState));
		g_NomalParticleSystem.Init(g_setParticleState, filename_);
	}


	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	g_NomalParticleSystem.SetParticleSystemState(g_ParticleState4ImGui);
	//ファイルアクセス関連

	//Imguiウィンドウ設定
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.7f, 0.0f, 0.2f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.3f, 0.0f, 0.1f, 1.0f));
	ImGui::SetNextWindowPos(ImVec2(SCREEN_X - 220, 20), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ImVec2(200, 500), ImGuiSetCond_Once);


	ImGui::Begin("ParticleFile");

	//ファイル書き出し
	if (ImGui::Button("Export")) {
		g_NomalParticleSystem.FOutState();
	}

	//ファイル読み込み
	WIN32_FIND_DATA win32fd;

	std::string DirectoryName = ".\\InPutData\\*.txt";//読み込むフォルダのディレクトリと拡張子指定
	HANDLE hFind;//ファイル操作ハンドル

	if (ImGui::TreeNode("Import")) {//Imguiのツリーが開かれたら
		hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//ファイルが存在するか確認
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (ImGui::Button(win32fd.cFileName)) {//発見したファイルをボタンとして表示
					//ボタンが押されたらそのファイルを読み込み反映
					g_NomalParticleSystem.FInState(win32fd.cFileName);
				}
			} while (FindNextFile(hFind, &win32fd));//次のファイルを捜索
		}
		ImGui::TreePop();
		FindClose(hFind);

	}

	//テクスチャ読み込み
	DirectoryName = ".\\InPutData\\*.png";
	if (ImGui::TreeNode("inputTexture")) {//Imguiのツリーが開かれたら
		hFind = FindFirstFile(DirectoryName.c_str(), &win32fd);//ファイルが存在するか確認
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (ImGui::Button(win32fd.cFileName)) {//発見したファイルをボタンとして表示
					//ボタンが押されたらそのファイルを読み込み反映
					g_NomalParticleSystem.FInTex(win32fd.cFileName);
				}
			} while (FindNextFile(hFind, &win32fd));//次のファイルを捜索
		}
		ImGui::TreePop();
		FindClose(hFind);

	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

//==============================================================================
//!	@fn		ImGuiReflection
//!	@brief	ゲーム終了処理
//!	@param　	
//!	@retval	なし
//==============================================================================
bool isPartilceStateEqual() {
	double sumState1, sumState2;


	//要修正
	//全部のメンバの数値を加算して値を比較
	sumState1 = g_setParticleState.m_PositionX + g_setParticleState.m_PositionY + g_setParticleState.m_PositionZ
		+ g_setParticleState.m_AngleRangeX + g_setParticleState.m_AngleRangeY + g_setParticleState.m_AngleRangeZ
		+ g_setParticleState.m_Color[0] + g_setParticleState.m_Color[1] + g_setParticleState.m_Color[2] + g_setParticleState.m_Color[3]
		+ g_setParticleState.m_ParticleNum + g_setParticleState.m_MaxLifeTime + g_setParticleState.m_DuaringTime
		+ g_setParticleState.m_Size + g_setParticleState.m_Speed + g_setParticleState.isLooping
		+ g_setParticleState.m_AngleX + g_setParticleState.m_AngleY + g_setParticleState.m_AngleZ;

	sumState2 = g_ParticleState4ImGui.m_PositionX + g_ParticleState4ImGui.m_PositionY + g_ParticleState4ImGui.m_PositionZ
		+ g_ParticleState4ImGui.m_AngleRangeX + g_ParticleState4ImGui.m_AngleRangeY + g_ParticleState4ImGui.m_AngleRangeZ
		+ g_ParticleState4ImGui.m_Color[0] + g_ParticleState4ImGui.m_Color[1] + g_ParticleState4ImGui.m_Color[2] + g_ParticleState4ImGui.m_Color[3]
		+ g_ParticleState4ImGui.m_ParticleNum + g_ParticleState4ImGui.m_MaxLifeTime + g_ParticleState4ImGui.m_DuaringTime
		+ g_ParticleState4ImGui.m_Size + g_ParticleState4ImGui.m_Speed + g_ParticleState4ImGui.isLooping
		+ g_ParticleState4ImGui.m_AngleX + g_ParticleState4ImGui.m_AngleY + g_ParticleState4ImGui.m_AngleZ;

	if (sumState1 == sumState2) {
		return true;
	}
	return false;
}
//******************************************************************************
//	End of file.
//******************************************************************************
