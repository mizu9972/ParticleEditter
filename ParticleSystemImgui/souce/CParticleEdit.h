#pragma once

#include "ParticleSystemParent.h"
#include "CParticle.h"
#include "CBillBoard.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

//パーティクルシステムのエディタークラス
//ImGuiを利用してパーティクルシステムのステータスを表示・操作する
class ParticleEditor {
private:
	ParticleSystemParent m_ParticleSystems;//パーティクルシステム親オブジェクト

	ParticleSystem* m_ViewParticleSystem = nullptr;//設定画面を表示するパーティクルシステム

	//出力ファイル名
	char m_FileName[64];

	//ホーミングターゲット
	CBillBoard m_TargetBillBoard;
	float m_TargetPosf[3] = { 0.0f,0.0f,0.0f };
	bool isDrawTargetObj = false;


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

	//パーティクルシステム操作
	void AddParticleSystem(t_ParticleSystemState* setState = nullptr);
	void DeleteParticleSystems();//全削除

	//ファイル入出力
	bool InputData(const char* FileName_);
	void OutputData(char* FileName_);
};
