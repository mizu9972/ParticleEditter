#pragma once
#include	<directxmath.h>
#include	<string.h>

#include "CTimer.h"
#include "C2DQuadtex.h"

using namespace DirectX;

//�p�[�e�B�N���V�X�e���ݒ�\����
//�蓮�Őݒ肵�Ċ��蓖�Ă�
typedef struct {
	float m_PositionX;//���W
	float m_PositionY;
	float m_PositionZ;

	int m_AngleX;
	int m_AngleY;
	int m_AngleZ;

	int m_AngleRangeX;//����������p�x�͈̔�
	int m_AngleRangeY;
	int m_AngleRangeZ;

	float m_DuaringTime;//��������
	float m_Size;//�傫��
	float m_MaxLifeTime;//�Œ���������
	float m_Speed;//���x
	int m_ParticleNum;//��������p�[�e�B�N����

	float m_Color[4];//�p�[�e�B�N���̐F
	bool isLooping;
}t_ParticleSystemState;

class ParticleSystem {
private:
	//��������p�[�e�B�N���̏��
	//�����Őݒ肳���
	struct m_Particles {
		float DelayTime;//���ˑҋ@����
		float LifeTime;//��������
		XMFLOAT4X4 Matrix;//�s��
		bool isAlive;//�������Ă��邩
		bool isWaiting;//�����ҋ@����
	};
	float NowTime;
	C2DQuadTex m_texquad;
	const char* m_TexName;
protected:
	t_ParticleSystemState* m_ParticleStateZero = NULL;//�����l�ۑ��p
	t_ParticleSystemState m_ParticleState;//�p�[�e�B�N���V�X�e���ݒ�
	m_Particles* Particles = NULL;
	int ParticlesDeathCount;
	float m_SystemLifeTime;//�p�[�e�B�N���V�X�e���c�莞��
	bool isSystemActive = false;
public:
	ParticleSystem() {};
	ParticleSystem(t_ParticleSystemState ParticleState_) {
		memcpy(m_ParticleStateZero, &ParticleState_, sizeof(t_ParticleSystemState));
	};
	~ParticleSystem() {
		if (Particles != NULL) {
			Particles = NULL;
			delete[] Particles;
		}
	};

	//��{�������\�b�h
	void Init(t_ParticleSystemState ParticleState_, const char* filename);//������
	void ZeroInit();//�R���X�g���N�^�Ő��l�ݒ肵���ꍇ�A�������̏�Ԃɏ������ł��� (�f�t�H���g�ɖ߂�����(������)�p)
	void Start();//�p�[�e�B�N�������J�n
	void Update();
	void Draw();
	void UnInit();
	//�t�@�C�����o�̓��\�b�h
	bool FInState(const char* FileName_);
	void FOutState();
	bool FInTex(const char* FileName_);

	//�A�N�Z�X���\�b�h
	//setter
	void SetSize(float Size_);
	void SetLifeTime(float LifeTime_);
	void SetSpeed(float Speed_);
	void SetFileName(const char* FileName_);
	void SetParticleSystemState(t_ParticleSystemState SetState_);

private:
	void RefParticle();//�p�[�e�B�N���̏��X�V
};