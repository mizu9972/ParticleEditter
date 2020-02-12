#pragma once

#include <unordered_map>

#include "Lib/stdafx.h"
#include "Lib/CBillBoard.h"
#include "CModel.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include "ImGui/ImGuizmo.h"

//パーティクルシステムのエディタークラス
//ImGuiを利用してパーティクルシステムのステータスを表示・操作する
class ParticleEditor {
private:
	ParticleSystemParent m_ParticleSystems;//パーティクルシステム親オブジェクト

	ParticleSystem* m_ViewParticleSystem = nullptr;//設定画面を表示するパーティクルシステム

	XMFLOAT4X4 m_SkyboxMatrix;
	std::unordered_map<const char*, CModel*> m_SkyBoxes;
	CModel* m_ViewSkybox;
	//出力ファイル名
	char m_FileName[64];

	//ホーミングターゲット
	CBillBoard m_TargetBillBoard;
	//オブジェクト
	CModel* m_Cube;
	XMFLOAT4X4 m_CubeMat = {
		0.0f,0.0f,0.0f,0.0f,
		0.0f,0.0f,0.0f,0.0f,
		0.0f,0.0f,0.0f,0.0f,
		0.0f,0.0f,0.0f,0.0f
	};

	float m_TargetPosf[3] = { 0.0f,0.0f,0.0f };
	bool isDrawTargetObj = false;

	ID3D11ShaderResourceView* m_test;

	ParticleEditor() {};
	~ParticleEditor() = default;

	bool isActive = true;

	//ImGui表示設定
	void ImGuiDrawMain();
	void ImGuiDrawofParticleSystem(ParticleSystem* pParticleSystem_);

	enum class WARNING_REACTION{
		OK,      //OK押された
		CANCEL,  //Cancel押された
		DONT_PUSH//押されていない
	};
	WARNING_REACTION ImGuiWarningText(const char* text);//警告表示


	//ImGuizmo
	float cameraView[16] =
	{ 1.f, 0.f, 0.f, 0.f,
	  0.f, 1.f, 0.f, 0.f,
	  0.f, 0.f, 1.f, 0.f,
	  0.f, 0.f, 0.f, 1.f };

	float cameraProjection[16];

	//マニピュレータ
	void EditTransform(t_ParticleSystemState* ViewState);
public:
	ParticleEditor(const ParticleEditor&) = delete;
	ParticleEditor(ParticleEditor&&) = delete;
	ParticleEditor operator = (const ParticleEditor&) = delete;
	ParticleEditor operator = (ParticleEditor&&) = delete;

	static ParticleEditor* getInstance() {
		static ParticleEditor instance;
		return &instance;
	}

	//基本処理
	void Init(unsigned int Width, unsigned int Height, ID3D11Device* device, ID3D11DeviceContext* devicecontext);
	void UnInit();
	void Update();
	void Draw();

	//パーティクルシステム操作
	void AddParticleSystem(t_ParticleSystemState* setState = nullptr);
	void DeleteParticleSystems();//全削除

	//ファイル入出力
	bool InputData(const char* FileName_);
	void OutputData(char* FileName_);
};
