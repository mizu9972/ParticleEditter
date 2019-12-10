#pragma once

#include "ParticleSystemParent.h"
#include "CParticle.h"
#include "CBillBoard.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

//パーティクルシステムのエディタークラス
//ImGuiを利用してパーティクルシステムのステータスを表示・操作する
class ParticleEditor {
private:
	ParticleSystemParent m_ParticleSystems;//パーティクルシステム親オブジェクト

	ParticleSystem* m_ViewParticleSystem = nullptr;//設定画面を表示するパーティクルシステム

	XMFLOAT3 m_TargetPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	float m_TargetPosf[3] = { 0.0f,0.0f,0.0f };
	CBillBoard m_TargetBillBoard;

	ParticleEditor() {};
	~ParticleEditor() = default;

	bool isActive = true;
	//ImGui表示設定
	void ImGuiDrawMain();
	void ImGuiDrawofParticleSystem(ParticleSystem* pParticleSystem_);
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
	void Init();
	void UnInit();
	void Update();
	void Draw();

	//パーティクルシステム全削除
	void DeleteParticleSystems();

	//ファイル入出力
	bool InputData(const char* FileName_);
	void OutputData();
};
