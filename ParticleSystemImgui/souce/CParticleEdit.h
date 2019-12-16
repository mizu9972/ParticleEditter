#pragma once

#include "ParticleSystemParent.h"
#include "CParticle.h"
#include "CBillBoard.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

//�p�[�e�B�N���V�X�e���̃G�f�B�^�[�N���X
//ImGui�𗘗p���ăp�[�e�B�N���V�X�e���̃X�e�[�^�X��\���E���삷��
class ParticleEditor {
private:
	ParticleSystemParent m_ParticleSystems;//�p�[�e�B�N���V�X�e���e�I�u�W�F�N�g

	ParticleSystem* m_ViewParticleSystem = nullptr;//�ݒ��ʂ�\������p�[�e�B�N���V�X�e��

	//�o�̓t�@�C����
	char m_FileName[64];

	//�z�[�~���O�^�[�Q�b�g
	CBillBoard m_TargetBillBoard;
	float m_TargetPosf[3] = { 0.0f,0.0f,0.0f };
	bool isDrawTargetObj = false;


	ParticleEditor() {};
	~ParticleEditor() = default;

	bool isActive = true;

	//ImGui�\���ݒ�
	void ImGuiDrawMain();
	void ImGuiDrawofParticleSystem(ParticleSystem* pParticleSystem_);

	enum class WARNING_REACTION{
		OK,      //OK�����ꂽ
		CANCEL,  //Cancel�����ꂽ
		DONT_PUSH//������Ă��Ȃ�
	};
	WARNING_REACTION ImGuiWarningText(const char* text);//�x���\��
public:
	ParticleEditor(const ParticleEditor&) = delete;
	ParticleEditor(ParticleEditor&&) = delete;
	ParticleEditor operator = (const ParticleEditor&) = delete;
	ParticleEditor operator = (ParticleEditor&&) = delete;

	static ParticleEditor* getInstance() {
		static ParticleEditor instance;
		return &instance;
	}

	//��{����
	void Init();
	void UnInit();
	void Update();
	void Draw();

	//�p�[�e�B�N���V�X�e������
	void AddParticleSystem(t_ParticleSystemState* setState = nullptr);
	void DeleteParticleSystems();//�S�폜

	//�t�@�C�����o��
	bool InputData(const char* FileName_);
	void OutputData(char* FileName_);
};
