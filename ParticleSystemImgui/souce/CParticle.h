#pragma once
#include	<directxmath.h>
#include	<string.h>
#include    <vector>

#include "CBillBoard.h"
#include "Shader.h"
#include "Observer.h"
#include <wrl.h>

#define     FPS 60

using namespace DirectX;
class ParticleSystem;

//�p�[�e�B�N���V�X�e���ݒ�\����
//�蓮�Őݒ肵�Ċ��蓖�Ă�
//�ꕔ�̐��l�͓��I�ɕύX���Ă��������f���ꂸ�A�p�[�e�B�N������������I����Ă��甽�f�����
typedef struct {
	char m_Name[64]         = "";//���O
	char m_TextureName[512] = "assets/ParticleTexture/particle.png";//�e�N�X�`���̖��O

	//���W
	float m_PositionX       = 0;
	float m_PositionY       = 0;
	float m_PositionZ       = 0;

	//�p�x
	int m_AngleX            = 0;
	int m_AngleY            = 0;
	int m_AngleZ            = 0;

	int m_AngleRange        = 360;//����������p�x�͈̔�

	float m_DuaringTime     = 10;//��������
	float m_Size            = 20;//�傫��
	float m_MaxLifeTime     = 1;//�Œ���������
	float m_Speed           = 30;//���x
	int m_ParticleNum       = 256;//��������p�[�e�B�N���� //�������f����Ȃ�

	float m_Color[4]        = {1.0f,1.0f,1.0f,1.0f};//�p�[�e�B�N���̐F
	int m_RotateSpeed       = 1;//��]���x
	bool isChaser           = false;//�^�[�Q�b�g�֌������Ă����p�[�e�B�N�����[�h
	bool isActive           = true;//Start���\�b�h�Ɠ����ɋN�����邩�ǂ���(���̃p�[�e�B�N���V�X�e���̌ォ�甭��������ꍇ��false)
	bool isLooping          = true;//���[�v���邩�ǂ���
	bool isGPUParticle      = false;//GPU�p�[�e�B�N��ONOFF

	//ParticleSystem* m_NextParticleSystem = nullptr;
	int m_SystemNumber;//���g�̔ԍ�(map�ŊǗ�����key�ɂȂ�)
	int m_NextSystemNumber = -1;
}t_ParticleSystemState;


class ParticleSystem:public Subject {
private:
	//��������p�[�e�B�N���̏��
	//�����Őݒ肳���
	struct m_Particles {
		float DelayTime;  //���ˑҋ@����
		float LifeTime;   //��������
		XMFLOAT4X4 Matrix;//�s��
		bool isAlive;     //�������Ă��邩
		bool isWaiting;   //�����ҋ@����
		int ZAngle;       //��]
		float CountTime;  //�o�ߎ���
		int RandNum;      //�����_���ɐݒ肳��鐔�l
	};

	//�R���s���[�g�V�F�[�_�[�ɑ���l
	struct m_ParticleSRVState {
		int isInitialied = 0;
	};
	m_ParticleSRVState InState;

	//�R���s���[�g�V�F�[�_�[���Ōv�Z����A�Ԃ��Ă���l
	struct m_ParticleUAVState {
		XMFLOAT4X4 Matrix;
		float DelayTime   = 0;      //���ˑҋ@����
		float LifeTime    = 0;      //��������
		float CountTime   = 0;      //�o�ߎ���
		int isAlive       = 1;      //�������Ă��邩
		int isWaiting     = 0;      //�����ҋ@����
		int isInitialized = 0;		//�������ς݂�
		int ZAngle        = 0;      //��]�p�x
		int RandNum       = 0;      //�����_���ɐݒ肳��鐔�l
	};
	//std::vector<m_ParticleUAVState*> m_ParticleUAVvec;
	m_ParticleUAVState* OutState;

	//�p�[�e�B�N���S�̋��ʂ̃p�����[�^
	//�R���X�^���g�o�b�t�@�ɗ���
	struct m_ConstantBufferParticle {
		XMFLOAT4 iPosition;//�S�̂̈ʒu
		XMINT4 iAngle;//�p�x
		int iAngleRange;//���˔͈�
		float iDuaringTime;//�p������
		float iMaxLifeTime;//�ő吶������
		float iSpeed;//���x
		int iRotateSpeed;//��]���x
		int isActive;//�L�����ǂ���
		int isLooping;//���[�v���邩�ǂ���
		int iParticleNum;//�p�[�e�B�N���̌�
		float iTime;//�o�ߎ���
		XMFLOAT3 iTargetPosition;//�ǂ�������^�[�Q�b�g�̍��W
		int isChaser;

		//�o�C�g�������p
		float Padding = 0;
		float Padding1 = 0;
		float Padding2 = 0;
	};
	m_ConstantBufferParticle m_CbParticle;

	float NowTime;
	CBillBoard m_BillBoard;


	XMFLOAT3 m_TargetPos;
	XMFLOAT4 m_Quaternion; //�N�H�[�^�j�I��
protected:
	t_ParticleSystemState* m_ParticleStateZero = nullptr;//�����l�ۑ��p
	t_ParticleSystemState m_ParticleState;//�p�[�e�B�N���V�X�e���ݒ�
	m_Particles* Particles = nullptr;

	std::vector<m_Particles> m_ParticleVec;//�����p�[�e�B�N���z��

	int m_ParticleNum;//�p�[�e�B�N�����ۑ��p
	int m_MaxParticleNum;//�p�[�e�B�N���ő吶����
	int ParticlesDeathCount;//���S�p�[�e�B�N���J�E���g
	float m_SystemLifeTime;//�p�[�e�B�N���V�X�e���c�莞��
	bool isSystemActive = false;

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	//�R���s���[�g�V�F�[�_�[�֘A
	ID3D11ComputeShader* m_ComputeShader          = nullptr;//�R���s���[�g�V�F�[�_�[
	//ComPtr<ID3D11ComputeShader> m_ComputeShader = nullptr;//�R���s���[�g�V�F�[�_�[
	ID3D11ComputeShader* m_InitComputeShader      = nullptr;//�������R���s���[�g�V�F�[�_�[
	ID3D11Buffer* m_pBuf                          = nullptr;//���̓o�b�t�@
	ID3D11Buffer* m_pResult                       = nullptr;//�o�̓o�b�t�@
	ID3D11Buffer* m_ConstantBuffer                = nullptr;//�R���X�^���g�o�b�t�@
	ID3D11Buffer* getbuf = nullptr;//�o�b�t�@�R�s�[�p
	D3D11_MAPPED_SUBRESOURCE m_MappedSubResource;//�R���s���[�g�V�F�[�_�[����Ԃ��Ă��鐔�l
	ID3D11ShaderResourceView* m_pSRV              = nullptr;//�V�F�[�_�[���\�[�X�r���[
	ID3D11UnorderedAccessView* m_pUAV             = nullptr;//�A���I�[�_�[�h�A�N�Z�X�r���[
public:
	ParticleSystem() {};
	ParticleSystem(t_ParticleSystemState ParticleState_) {
		memcpy(m_ParticleStateZero, &ParticleState_, sizeof(t_ParticleSystemState));
	};
	~ParticleSystem() {
		UnInit();
	};
	
	//��{�������\�b�h

	//������
	void Init();
	void Init(t_ParticleSystemState* ParticleState_);
	void Init(t_ParticleSystemState ParticleState_, const char* filename, ID3D11Device* device);
	void ZeroInit();//�R���X�g���N�^�Ő��l�ݒ肵���ꍇ�A�������̏�Ԃɏ������ł���
	void InitComputeShader();//�R���s���[�g�V�F�[�_�[�̏�����

	//�X�V����
	void Update();
	void (ParticleSystem::*fpUpdateFunc)() = &ParticleSystem::UpdateNomal;//�֐��|�C���^
	void UpdateNomal();
	void UpdateComputeShader();

	XMFLOAT4 RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d);
	//�p�[�e�B�N�������J�n
	void Start();//�p�[�e�B�N�������J�n
	void StartGPUParticle();
	
	//�`��
	void Draw(ID3D11DeviceContext* device);
	void (ParticleSystem::*fpDrawFunc)(ID3D11DeviceContext* device) = &ParticleSystem::DrawNomal;//�֐��|�C���^
	void DrawNomal(ID3D11DeviceContext* device);
	void GPUDraw(ID3D11DeviceContext* device);

	//�I������
	void UnInit();

	//�p�[�e�B�N������
	void AddParticle(m_Particles* AddParticle);
	void AddGPUParticle(m_Particles* AddParticle);
	void ChangeGPUParticleMode(bool isGPUMode = true);//GPU�p�[�e�B�N�����[�h�ύX

	//�t�@�C�����o�̓��\�b�h
	bool FInState(const char* FileName_);
	void FOutState();
	bool FInTex(const char* FileName_);

	//�A�N�Z�X���\�b�h
	//setter
	void SetSize(float Size_);
	void SetLifeTime(float LifeTime_);
	void SetSpeed(float Speed_);
	void SetName(const char*  setName);
	void SetFileName(const char* FileName_);
	ParticleSystem& SetActive(bool set);
	void SetTargetPos(float x, float y, float z);
	void SetParticleSystemState(t_ParticleSystemState* SetState_);
	void SetNextParticleSystem(ParticleSystem* next);
	void SetNextParticleSystem(int NextNumber);
	ParticleSystem& SetComputeShader(ID3D11ComputeShader* setShader);
	ParticleSystem& setSystemNumber(int setNumber);
	//getter
	t_ParticleSystemState GetState();
	char* getName();
	int getSystemNumber();
	int getNextSystemNumber();
};