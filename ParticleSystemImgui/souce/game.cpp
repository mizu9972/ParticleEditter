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
#include "Lib/ParticleMathUtil.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "CParticleEdit.h"
//パーティクルで使用するテクスチャの名前
constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/particle.png";
//-----------------------------------------------------------------------------
// グローバル変数
//-----------------------------------------------------------------------------
extern int g_nCountFPS;//FPS

//-----------------------------------------------------------------------------
// プロトタイプ宣言
//-----------------------------------------------------------------------------

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

	ParticleEditor::getInstance()->Init(width, height, CDirectXGraphics::GetInstance()->GetDXDevice(),CDirectXGraphics::GetInstance()->GetImmediateContext());//エディター初期化

	// カメラ変換行列初期化
	// プロジェクション変換行列初期化
	XMFLOAT3 eye =	  {0,0,50};				// 視点
	XMFLOAT3 lookat = { 0,0,0 };			// 注視点
	XMFLOAT3 up = {0,1,0};					// 上向きベクトル

	CCamera::GetInstance()->Init(1.0f, 10000.0f, XM_PIDIV2, SCREEN_X, SCREEN_Y, eye, lookat, up);

	CCamera::GetInstance()->SetRadius(50.0f);
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
	ParticleEditor::getInstance()->Update();
	CCamera::GetInstance()->UpdateSphereCamera();
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

	ParticleEditor::getInstance()->Draw();

	bool show_demo_window = true;
	bool show_another_window = false;

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
	ImGui::DestroyContext();
	ParticleEditor::getInstance()->DeleteParticleSystems();
	ParticleEditor::getInstance()->UnInit();
	DX11Uninit();
}

//******************************************************************************
//	End of file.
//******************************************************************************
