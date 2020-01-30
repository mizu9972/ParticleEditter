#pragma once
#include	<directxmath.h>
#include	<string.h>
#include    <vector>

#include "CBillBoard.h"
#include "Observer.h"
#include <wrl.h>

#define     FPS 60

using namespace DirectX;
template<typename ComPtrT>
using ComPtr = Microsoft::WRL::ComPtr<ComPtrT>;

//�\�t�g�p�[�e�B�N���p�R���X�^���g�o�b�t�@�\����
struct ConstantBufferSoftParticle {
	UINT iViewPort[2] = { 1600,900 };
	float iZfar = 0.1f;
	float iZVolume = 110.0f;
};

//�p�[�e�B�N���V�X�e���ݒ�\����
//�蓮�Őݒ肵�Ċ��蓖�Ă�
//�ꕔ�̐��l�͓��I�ɕύX���Ă��������f���ꂸ�A�p�[�e�B�N������������I����Ă��甽�f�����
typedef struct {
	char m_Name[64]         = "";//���O
	char m_TextureName[512] = "assets/ParticleTexture/particle.png";//�e�N�X�`���̖��O
	
	float m_Position[3]     = { 0,0,0 };//���W
	int m_Angle[3]          = { 0,0,0 };//�p�x
	int m_AngleRange        = 360;//����������p�x�͈̔�

	float m_StartDelayTime  = 0;//�J�n�x������
	float m_DuaringTime     = 10;//��������
	float m_Size            = 20;//�傫��
	float m_MaxLifeTime     = 10;//�Œ���������
	float m_Speed           = 50;//���x
	float m_Accel           = 0;//�����x
	float m_MinSpeed        = 0;//�ŏ����x
	float m_MaxSpeed        = 100;//�ő呬�x
	int m_ParticleNum       = 256;//��������p�[�e�B�N���� //�������f����Ȃ�

	float m_Color[4]        = {1.0f,1.0f,1.0f,1.0f};//�p�[�e�B�N���̐F
	int m_RotateSpeed       = 1;//��]���x
	bool isChaser           = false;//�^�[�Q�b�g�֌������Ă����p�[�e�B�N�����[�h
	bool isActive           = true;//Start���\�b�h�Ɠ����ɋN�����邩�ǂ���(���̃p�[�e�B�N���V�X�e���̌ォ�甭��������ꍇ��false)
	bool isEmitting         = false;//���p�[�e�B�N�����甭���������邩�ǂ���
	bool isLooping          = true;//���[�v���邩�ǂ���
	bool isGPUParticle      = false;//GPU�p�[�e�B�N��ONOFF
	bool UseGravity         = false;//�d�͗L��
	bool isSoftParticle		= false;//�\�t�g�p�[�e�B�N���ɂ��邩�ǂ���
	ConstantBufferSoftParticle m_CBSoftParticleState;//�R���X�^���g�o�b�t�@�ɗ��p����\����

	float m_Gravity[3]      = { 0,0,0 };//�d��

	//�z�[�~���O�p�x����
	int m_MinChaseAngle     = 0;
	int m_MaxChaseAngle     = 5;

	int m_SystemNumber;//���g�̔ԍ�(map�ŊǗ�����key�ɂȂ�)
	int m_NextSystemNumber  = -1;//���ɔ���������p�[�e�B�N���̔ԍ�
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
		XMFLOAT4 iPosition;      //�S�̂̈ʒu
		XMINT4 iAngle;           //�p�x
		int iAngleRange;         //���˔͈�
		float iDuaringTime;      //�p������
		float iDelayTime;        //�x������
		float iMaxLifeTime;      //�ő吶������
		float iSpeed;            //���x
		float iAccel;            //�����x
		float iMinSpeed;         //�ŏ����x
		float iMaxSpeed;         //�ő呬�x
		int iRotateSpeed;        //��]���x
		int isActive;            //�L�����ǂ���
		int isLooping;           //���[�v���邩�ǂ���
		int iParticleNum;        //�p�[�e�B�N���̌�
		float iTime;             //�o�ߎ���
		XMFLOAT3 iTargetPosition;//�ǂ�������^�[�Q�b�g�̍��W
		int isChaser;            //�^�[�Q�b�g��ǂ������邩
		int iMinChaseAngle;      //�ǂ�������ŏ��p�x
		int iMaxChaseAngle;      //�ǂ�������ő�p�x

		int UseGravity;          //�d�͂𗘗p���邩
		XMFLOAT3 iGravity;       //�d��
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
	m_ParticleUAVState* OutState;

	float NowTime;
	CBillBoard m_BillBoard;


	XMFLOAT3 m_TargetPos;
	XMFLOAT4 m_Quaternion; //�N�H�[�^�j�I��
protected:
	t_ParticleSystemState* m_ParticleStateZero = nullptr;//�����l�ۑ��p
	t_ParticleSystemState m_ParticleState;//�p�[�e�B�N���V�X�e���ݒ�
	m_Particles* Particles = nullptr;

	std::vector<m_Particles> m_ParticleVec;//�����p�[�e�B�N���z��
	std::vector<int> m_NextParticleNumberVector;

	int m_ParticleNum;//�p�[�e�B�N�����ۑ��p
	int m_MaxParticleNum;//�p�[�e�B�N���ő吶����
	int ParticlesDeathCount;//���S�p�[�e�B�N���J�E���g
	float m_SystemLifeTime;//�p�[�e�B�N���V�X�e���c�莞��
	bool isEmitting = true;
	bool isSystemActive = false;
	bool isUpdateActive = true;
	bool isDrawActive = true;
	
	//�R���s���[�g�V�F�[�_�[�֘A
	ID3D11Device* m_Device                        = nullptr;
	ID3D11DeviceContext* m_DeviceContext          = nullptr;
	ID3D11ComputeShader* m_ComputeShader          = nullptr;//�R���s���[�g�V�F�[�_�[(ParticleSystemParent�ŏ��������ꂽ���̂��󂯎��)
	ID3D11ComputeShader* m_InitComputeShader      = nullptr;//�������p�R���s���[�g�V�F�[�_�[(ParticleSystemParent�ŏ��������ꂽ���̂��󂯎��)
	ID3D11Buffer* m_pResult                       = nullptr;//�o�̓o�b�t�@
	ID3D11Buffer* m_ConstantBufferSoftParticle    = nullptr;//�\�t�g�p�[�e�B�N���p�R���X�^���g�o�b�t�@
	ID3D11Buffer* getbuf                          = nullptr;//�o�b�t�@�R�s�[�p
	ID3D11Buffer* m_pbuf                          = nullptr;//�V�F�[�_�[���\�[�X�r���[�p�o�b�t�@
	ID3D11ShaderResourceView* m_pSRV              = nullptr;//�V�F�[�_�[���\�[�X�r���[
	ID3D11UnorderedAccessView* m_pUAV             = nullptr;//�A���I�[�_�[�h�A�N�Z�X�r���[
	D3D11_MAPPED_SUBRESOURCE m_MappedSubResource;//�R���s���[�g�V�F�[�_�[����Ԃ��Ă��鐔�l

	//com�|�C���^�錾
	ComPtr<ID3D11Buffer> m_CpResult               = nullptr;//�o�̓o�b�t�@
	ComPtr<ID3D11Buffer> m_CpCBufferSoftParticle  = nullptr;//�R���X�^���g�o�b�t�@
	ComPtr<ID3D11Buffer> m_CpGetBuf               = nullptr;//�o�b�t�@�R�s�[�p
	ComPtr<ID3D11Buffer> m_CpBuf                  = nullptr;//�V�F�[�_�[���\�[�X�r���[�p�o�b�t�@
	ComPtr<ID3D11ShaderResourceView> m_CpSRV      = nullptr;//�V�F�[�_�[���\�[�X�r���[
	ComPtr<ID3D11UnorderedAccessView> m_CpUAV     = nullptr;//�A���I�[�_�[�h�A�N�Z�X�r���[
public:
	ParticleSystem() {};
	ParticleSystem(t_ParticleSystemState ParticleState_) {
		memcpy(m_ParticleStateZero, &ParticleState_, sizeof(t_ParticleSystemState));
	};
	~ParticleSystem() {
		UnInit();
	};
	
	//�R���s���[�g�V�F�[�_�[�̎�ރ��X�g
	enum class eComputeShaderType{
		INIT,
		UPDATE,
	};

	//��{�������\�b�h

	//������
	ParticleSystem& Init(ID3D11Device* device, ID3D11DeviceContext* devicecontext,t_ParticleSystemState* ParticleState_ = nullptr, const char* filename = nullptr);
	void ZeroInit();//�R���X�g���N�^�Ő��l�ݒ肵���ꍇ�A�������̏�Ԃɏ������ł���
	void InitComputeShader();//�R���s���[�g�V�F�[�_�[�̏�����

	//�X�V����
	void Update();
	void (ParticleSystem::*fpUpdateFunc)() = &ParticleSystem::UpdateNomal;//�֐��|�C���^
	void UpdateNomal();
	void UpdateComputeShader();
	void UpdateSRV();

	XMFLOAT4 RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d);
	//�p�[�e�B�N�������J�n
	void Start();//�p�[�e�B�N�������J�n
	void (ParticleSystem::*fpStartFunc)() = &ParticleSystem::StartNomalParticle;
	void StartNomalParticle();
	void StartGPUParticle();
	
	//�`��
	void Draw(const XMFLOAT4X4& CameraMatrix);
	void (ParticleSystem::*fpDrawFunc)(const XMFLOAT4X4& CameraMatrix) = &ParticleSystem::DrawNomal;//�֐��|�C���^
	void DrawNomal(const XMFLOAT4X4& CameraMatrix);
	void GPUDraw(const XMFLOAT4X4& CameraMatrix);

	//�I������
	void UnInit();

	//�p�[�e�B�N������
	void AddParticle(m_Particles* AddParticle);
	void ChangeGPUParticleMode(bool isGPUMode = true);//GPU�p�[�e�B�N�����[�h�ύX
	void ChangeSoftParticleMode(bool isSoftParticle = true);//�\�t�g�p�[�e�B�N�����[�h�ؑ�

	//�t�@�C�����o�̓��\�b�h
	bool FInState(const char* FileName_);
	void FOutState();
	bool FInTex(const char* FileName_);

	//�A�N�Z�X���\�b�h
	//setter
	void SetName(const char*  setName);//���O
	ParticleSystem& SetActive(bool set);//�L������
	ParticleSystem& SetEmitte(bool set);
	void SetTargetPos(float x, float y, float z);//�^�[�Q�b�g���W
	void SetParticleSystemState(t_ParticleSystemState* SetState_);//�\���̏��S��
	void SetNextParticleSystem(int NextNumber);//���̃p�[�e�B�N���V�X�e���ԍ�
	void SetSoftPConstantBuffer(ConstantBufferSoftParticle* setState = nullptr);
	void SetisUpdateActive(bool active);
	void SetisDrawActive(bool active);
	ParticleSystem& SetViewPort(UINT* viewport);
	ParticleSystem& SetComputeShader(ID3D11ComputeShader* setShader, eComputeShaderType type);//�R���s���[�g�V�F�[�_�[
	ParticleSystem& setSystemNumber(int setNumber);//���g�̃p�[�e�B�N���V�X�e���ԍ�
	//getter
	float* getMatrixf16();
	t_ParticleSystemState GetState();//�\���̏��S��
	char* getName();//���O
	int getSystemNumber();//���g�̃p�[�e�B�N���V�X�e���ԍ�
	int getNextSystemNumber();//���̃p�[�e�B�N���V�X�e���ԍ�
	std::vector<int> getNextSystemNumbers();
	bool getisUpdateActive()const;
	bool getisDrawActive()const;
	float getLifeTime();
	ConstantBufferSoftParticle getCBSoftParticleState();
};