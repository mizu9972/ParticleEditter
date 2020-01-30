//�p�[�e�B�N���V�X�e���v���O����

//�w�b�_�[
#include <random>
#include <algorithm>
#include "CParticle.h"
#include "dx11mathutil.h"
#include "ParticleSystemUtility.h"

#define		SCREEN_X		1200
#define		SCREEN_Y		600
#define		PARTICLE_NUM_PER_THREAD (64) //csParticle.hlsl���Ɠ����萔��錾
#define		THREAD_NUM (256)

#ifndef     ALIGN16
#define     ALIGN16 _declspec(align(16))
#endif

#define STS_ifERROR_FUNCTION(x) if (x != true) MessageBox(NULL, "AnyFunction is Error", "Error", MB_OK)


constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/particle.png";
constexpr auto PARTICLE_PS_SHADER = "shader/psParticle.fx";
constexpr auto PARTICLE_VS_SHADER = "shader/vsPrticle.fx";

constexpr auto SOFTPARTICLE_PS_SHADER = "shader/psSoftParticle.fx";
constexpr auto SOFTPARTICLE_VS_SHADER = "shader/vsSoftParticle.fx";
/*------------------------
�P�ʍs��ɂ���
--------------------------*/
void DX11MatrixIdentity_subDefiner(DirectX::XMFLOAT4X4& mat) {

	ALIGN16 XMMATRIX mtx;

	mtx = XMMatrixIdentity();

	XMStoreFloat4x4(&mat, mtx);
}

//���\�b�h

//�֐��|�C���^���p���Đ؂�ւ�鏈�����Ăяo�����\�b�h---------------------------
//�p�[�e�B�N�������J�n
void ParticleSystem::Start() {
	if (isEmitting == true && m_ParticleState.isEmitting == true) {
		return;
	}
	isEmitting = m_ParticleState.isEmitting;
	(this->*fpStartFunc)();
}

//�X�V����
void ParticleSystem::Update() {
	if (isUpdateActive == false) {
		return;
	}
	(this->*fpUpdateFunc)();
}

//�`�揈��
void ParticleSystem::Draw(const XMFLOAT4X4& CameraMatrix) {
	if (isDrawActive == false) {
		return;
	}

	(this->*fpDrawFunc)(CameraMatrix);

}
//--------------------------------------------------------------------

//������
ParticleSystem& ParticleSystem::Init(ID3D11Device* device, ID3D11DeviceContext* devicecontext,t_ParticleSystemState* ParticleState_, const char* filename) {
	//�p�[�e�B�N��������

	m_Device = device;
	m_DeviceContext = devicecontext;
	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
	}

	t_ParticleSystemState newState;
	//�����̃X�e�[�^�X��ݒ�
	if (ParticleState_ != nullptr) {
		newState = *ParticleState_;

	}
	SetParticleSystemState(&newState);
	InitComputeShader();

	if (filename != nullptr) {
		// �t�@�C������ۑ�
		strcpy_s(m_ParticleState.m_TextureName, filename);
	}

	ChangeSoftParticleMode(newState.isSoftParticle);

	bool sts = ParticleSystemUtility::CreateConstantBuffer(m_Device, sizeof(ConstantBufferSoftParticle), m_CpCBufferSoftParticle.GetAddressOf());
	STS_ifERROR_FUNCTION(sts);
	SetSoftPConstantBuffer();
	
	
	//�r���{�[�h������---------------------------------------------------------------------------------------------------------------
	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
	m_BillBoard.SetUV(u, v);
	m_BillBoard.LoadTexTure(m_ParticleState.m_TextureName);
	//----------------------------------------------------------------------------------------------------------------------------

	Start();
	return *this;
}

//�X�e�[�^�X������
void ParticleSystem::ZeroInit() {

	if (m_ParticleStateZero != NULL) {

		SetParticleSystemState(m_ParticleStateZero);

		Start();
	}

}

//�R���s���[�g�V�F�[�_�[������
void ParticleSystem::InitComputeShader() {

	//Com�|�C���^�ݒ�
	m_CpSRV.Attach(m_pSRV);
	m_CpUAV.Attach(m_pUAV);
	m_CpResult.Attach(m_pResult);
	m_CpCBufferSoftParticle.Attach(m_ConstantBufferSoftParticle);
	m_CpGetBuf.Attach(getbuf);
	m_CpBuf.Attach(m_pbuf);
}

//GPU�p�[�e�B�N���V�X�e���X�V
void ParticleSystem::UpdateComputeShader() {

	if (m_ParticleNum <= 0) {
		return;
	}
	NowTime = 1.0f / FPS;
	m_SystemLifeTime -= NowTime;
	//�p�[�e�B�N���I������
	if (isSystemActive == true) {
		if (m_SystemLifeTime <= 0) {
			if (m_NextParticleNumberVector.size() > 0) {
				//���̃p�[�e�B�N���J�n
				Notify(this);
			}
			if(m_ParticleState.isLooping == false){
				isSystemActive = false;
			}
			m_SystemLifeTime = m_ParticleState.m_DuaringTime + m_ParticleState.m_StartDelayTime;
		}
	}

	//�R���s���[�g�V�F�[�_�[�����s
	const UINT dispatchX = UINT(ceil(float(m_ParticleNum) / float(THREAD_NUM * PARTICLE_NUM_PER_THREAD)));
	ParticleSystemUtility::RunComputeShader(m_DeviceContext, m_ComputeShader, 1, m_CpSRV.GetAddressOf(), m_CpUAV.Get(), dispatchX, 1, 1);

	//�f�[�^�󂯎��
	m_DeviceContext->CopyResource(m_CpGetBuf.Get() , m_CpResult.Get());//�o�b�t�@�R�s�[
	m_DeviceContext->Map(m_CpGetBuf.Get(), 0, D3D11_MAP_READ, 0, &m_MappedSubResource);

	OutState = reinterpret_cast<m_ParticleUAVState*>(m_MappedSubResource.pData);//�f�[�^�l��

	m_DeviceContext->Unmap(m_CpGetBuf.Get(), 0);
}

void ParticleSystem::UpdateNomal() {

	m_Particles* p_PickParticle;//���삷��p�[�e�B�N���̃|�C���^�ۑ��p

	//float lng;
	XMFLOAT4 TargetQt;//�^�[�Q�b�g�����̎p��
	XMFLOAT3 TargetVector;

	float Speed_ = m_ParticleState.m_Speed;

	NowTime = 1.0f / FPS;

	//�p�[�e�B�N���X�V
	for (int ParticleNum = 0; ParticleNum < m_ParticleVec.size(); ParticleNum++) {

		if (m_ParticleState.isChaser) {

			//���W�ۑ�
			XMFLOAT3 ParticlePosition = XMFLOAT3(m_ParticleVec[ParticleNum].Matrix._41, m_ParticleVec[ParticleNum].Matrix._42, m_ParticleVec[ParticleNum].Matrix._43);
			
			//���݈ʒu����^�[�Q�b�g�����ւ̃x�N�g�������߂�
			TargetVector.x = m_TargetPos.x - m_ParticleVec[ParticleNum].Matrix._41;
			TargetVector.y = m_TargetPos.y - m_ParticleVec[ParticleNum].Matrix._42;
			TargetVector.z = m_TargetPos.z - m_ParticleVec[ParticleNum].Matrix._43;
			//DX11GetQtfromMatrix(m_ParticleVec[ParticleNum].Matrix, Quaternion);

			XMFLOAT3 ZDir = XMFLOAT3(m_ParticleVec[ParticleNum].Matrix._31, m_ParticleVec[ParticleNum].Matrix._32, m_ParticleVec[ParticleNum].Matrix._33);//�~�T�C���̕����x�N�g��(Z����)

			//���K��
			DX11Vec3Normalize(TargetVector, TargetVector);
			DX11Vec3Normalize(ZDir, ZDir);
			DX11GetQtfromMatrix(m_ParticleVec[ParticleNum].Matrix, m_Quaternion);

			float Dot;//�Q�{�̃x�N�g���̓��ϒl
			TargetQt = RotationArc(ZDir, TargetVector, Dot);//�Q�{�̃x�N�g������ׂ��p�x�ƃN�H�[�^�j�I�������߂�
			float AngleDiff = acosf(Dot);//���W�A���p�x
			float AngleMax = XM_PI * m_ParticleState.m_MaxChaseAngle / 180.0f;
			float AngleMin = XM_PI * m_ParticleState.m_MinChaseAngle / 180.0f;
			//�p�[�e�B�N���̎p�������肷��
			if (AngleMin > AngleDiff) {

			}else if (AngleMax >= AngleDiff) {
				//�p�x�̍����X�V�ł���p�x���傫������������
				DX11QtMul(m_Quaternion, m_Quaternion, TargetQt);
			}
			else {
				float t = AngleMax / AngleDiff;

				XMFLOAT4 toqt;
				DX11QtMul(toqt, m_Quaternion, TargetQt);
				DX11QtSlerp(m_Quaternion, toqt, t, m_Quaternion);
			}

			//�N�H�[�^�j�I�����s��ɂ���
			//���݂̎p�����N�H�[�^�j�I���ɂ���
			DX11MtxFromQt(m_ParticleVec[ParticleNum].Matrix, m_Quaternion);

			m_ParticleVec[ParticleNum].Matrix._41 = ParticlePosition.x;
			m_ParticleVec[ParticleNum].Matrix._42 = ParticlePosition.y;
			m_ParticleVec[ParticleNum].Matrix._43 = ParticlePosition.z;

		}

		m_ParticleVec[ParticleNum].CountTime += NowTime;

		//�����x�v�Z
		Speed_ = m_ParticleState.m_Speed + m_ParticleState.m_Accel * m_ParticleVec[ParticleNum].CountTime * m_ParticleVec[ParticleNum].CountTime;
		if (m_ParticleState.m_Accel != 0.0f) {
			Speed_ = min(Speed_, m_ParticleState.m_MaxSpeed);
			Speed_ = max(Speed_, m_ParticleState.m_MinSpeed);
		}
		//���x���ړ�������
		m_ParticleVec[ParticleNum].Matrix._41 += m_ParticleVec[ParticleNum].Matrix._31 * Speed_ * NowTime;
		m_ParticleVec[ParticleNum].Matrix._42 += m_ParticleVec[ParticleNum].Matrix._32 * Speed_ * NowTime;
		m_ParticleVec[ParticleNum].Matrix._43 += m_ParticleVec[ParticleNum].Matrix._33 * Speed_ * NowTime;

		if (m_ParticleState.UseGravity) {
			//�d��
			m_ParticleVec[ParticleNum].Matrix._41 += m_ParticleState.m_Gravity[0] / 100.0f * m_ParticleVec[ParticleNum].CountTime;
			m_ParticleVec[ParticleNum].Matrix._42 += m_ParticleState.m_Gravity[1] / 100.0f * m_ParticleVec[ParticleNum].CountTime;
			m_ParticleVec[ParticleNum].Matrix._43 += m_ParticleState.m_Gravity[2] / 100.0f * m_ParticleVec[ParticleNum].CountTime;
		}
		//�������Ԍ���
		m_ParticleVec[ParticleNum].LifeTime -= NowTime;

		m_ParticleVec[ParticleNum].ZAngle += m_ParticleState.m_RotateSpeed;
		m_ParticleVec[ParticleNum].ZAngle = m_ParticleVec[ParticleNum].ZAngle % 360;

		//���S����
		if (m_ParticleVec[ParticleNum].LifeTime <= 0) {
			m_ParticleVec[ParticleNum].isAlive = false;
			m_ParticleVec.erase(m_ParticleVec.begin() + ParticleNum);//�z�񂩂�폜
			m_ParticleVec.shrink_to_fit();//���������
			ParticlesDeathCount += 1;
		}
	}

	m_SystemLifeTime -= NowTime;

	//�p�[�e�B�N���I������
	if (m_SystemLifeTime <= 0) {
		if (isSystemActive != true) {
			return;
		}
		isSystemActive = false;

		if (m_NextParticleNumberVector.size() > 0) {
			//���̃p�[�e�B�N���J�n
			Notify(this);
		}

		if (m_ParticleState.isLooping == true) {//���[�v

			if (m_ParticleState.isActive == true) {
				Start();
			}

		}

	}
	if (isSystemActive == false) {
		return;
	}


	//�ҋ@���̃p�[�e�B�N��
	for (int ParticlesNum = 0; ParticlesNum < m_MaxParticleNum; ParticlesNum++) {

		p_PickParticle = &Particles[ParticlesNum];

		//�ҋ@���Ȃ�ҋ@���Ԍ���
		if (p_PickParticle->isWaiting == true) {
			p_PickParticle->DelayTime -= NowTime;

			//�ҋ@���Ԃ�0�ȉ��Ȃ犈����
			if (p_PickParticle->DelayTime <= 0) {
				p_PickParticle->CountTime = 0;
				p_PickParticle->isWaiting = false;
				p_PickParticle->isAlive = true;

				AddParticle(p_PickParticle);
			}
		}
	}
	
}

void ParticleSystem::UpdateSRV(){

	//���͗p�o�b�t�@���X�V
	InState.iPosition       = { m_ParticleState.m_Position[0],m_ParticleState.m_Position[1],m_ParticleState.m_Position[2],0 };
	InState.iAngle          = { 360 - m_ParticleState.m_Angle[0],360 - m_ParticleState.m_Angle[1],360 - m_ParticleState.m_Angle[2],0 };
	InState.iAngleRange     = m_ParticleState.m_AngleRange;
	InState.iDuaringTime    = m_ParticleState.m_DuaringTime;
	InState.iDelayTime      = m_ParticleState.m_StartDelayTime;
	InState.iMaxLifeTime    = m_ParticleState.m_MaxLifeTime;
	InState.iSpeed          = m_ParticleState.m_Speed;
	InState.iAccel          = m_ParticleState.m_Accel;
	InState.iMinSpeed       = m_ParticleState.m_MinSpeed;
	InState.iMaxSpeed       = m_ParticleState.m_MaxSpeed;
	InState.iRotateSpeed    = m_ParticleState.m_RotateSpeed;
	InState.isActive        = m_ParticleState.isActive;
	InState.isLooping       = m_ParticleState.isLooping;
	InState.iParticleNum    = m_ParticleState.m_ParticleNum;
	InState.iTime           = 1.0f / FPS;
	InState.iTargetPosition = m_TargetPos;
	InState.isChaser        = m_ParticleState.isChaser;
	InState.iMinChaseAngle  = m_ParticleState.m_MinChaseAngle;
	InState.iMaxChaseAngle  = m_ParticleState.m_MaxChaseAngle;
	InState.iGravity        = { m_ParticleState.m_Gravity[0],m_ParticleState.m_Gravity[1],m_ParticleState.m_Gravity[2] };
	InState.UseGravity      = m_ParticleState.UseGravity;

	if (m_CpSRV != nullptr) {
		m_CpSRV.Reset();
	}
	if (m_CpBuf != nullptr) {
		m_CpBuf.Reset();
	}
	ParticleSystemUtility::CreateStructuredBuffer(m_Device, sizeof(m_ParticleSRVState), 1, &InState, m_CpBuf.GetAddressOf());
	ParticleSystemUtility::CreateShaderResourceView(m_Device, m_CpBuf.Get(), m_CpSRV.GetAddressOf());
}

XMFLOAT4 ParticleSystem::RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d) {
	XMFLOAT3 Axis;//��
	XMFLOAT4 q; //�N�H�[�^�j�I��

	DX11Vec3Cross(Axis, v0, v1);

	DX11Vec3Dot(d, v0, v1);
	//�^�[�Q�b�g�̕����Ǝ��@���قƂ�ǈ�v�����Ƃ��A���ς̒l���P�𒴂���(-1�������)��������̂ŕ␳����
	if (d > 1.0) {
		d = 1.0;
	}
	if (d <= -1) {
		d = -1.0f;
	}

	float s = (float)sqrtf((1 + d) * 2);
	if (s == 0.0f) {
		DX11QtIdentity(q);//�^�[�Q�b�g��ǂ��z����
	}
	else {
		q.x = Axis.x / s;
		q.y = Axis.y / s;
		q.z = Axis.z / s;
		q.w = s / 2;
	}

	return q;
}

void ParticleSystem::StartNomalParticle() {
	//�p�[�e�B�N������
	if (Particles != NULL) {
		delete[] Particles;
	}
	Particles = new m_Particles[m_ParticleState.m_ParticleNum];
	isSystemActive = true;
	ParticlesDeathCount = 0;

	m_SystemLifeTime = m_ParticleState.m_DuaringTime + m_ParticleState.m_StartDelayTime;

	//�ύX���ꂽ�X�e�[�^�X���f
	{
		m_MaxParticleNum = m_ParticleState.m_ParticleNum;
	}
	//�p�[�e�B�N���ݒ�-------------------------------------------------------------------------
	srand((int)NowTime);

	m_Particles* p_PickParticle;//���삷��p�[�e�B�N���̃|�C���^�ۑ��p

	//�p�[�e�B�N���̏����ݒ肷��
	for (int ParticlesNum = 0; ParticlesNum < m_MaxParticleNum; ParticlesNum++) {

		p_PickParticle = &Particles[ParticlesNum];

		//�����܂ł̑ҋ@���Ԑݒ�
		//�p�[�e�B�N���̔������Ԃ��p�[�e�B�N�����œ������A���ꂼ�ꊄ�蓖�Ă�
		p_PickParticle->DelayTime = (m_ParticleState.m_DuaringTime / m_MaxParticleNum * ParticlesNum) + m_ParticleState.m_StartDelayTime;

		p_PickParticle->CountTime = 0;
		//�ҋ@����
		p_PickParticle->isAlive = false;
		p_PickParticle->isWaiting = true;
	}
	//--------------------------------------------------------------------------------------
}

void ParticleSystem::StartGPUParticle(){
	////GPU�p�[�e�B�N���ݒ�---------------------------------

	m_ParticleNum = m_ParticleState.m_ParticleNum;
	if (m_CpUAV != nullptr) {
		m_CpUAV.Reset();
	}
	if (m_CpResult != nullptr) {
		m_CpResult.Reset();
	}

	//���͗p�o�b�t�@���X�V
	UpdateSRV();
	//�o�͗p�o�b�t�@���X�V
	ParticleSystemUtility::CreateStructuredBuffer(m_Device, sizeof(m_ParticleUAVState), m_ParticleNum, nullptr, m_CpResult.GetAddressOf());
	ParticleSystemUtility::CreateUnOrderAccessView(m_Device, m_CpResult.Get(), m_CpUAV.GetAddressOf());

	//�������p�R���s���[�g�V�F�[�_�[���s
	ParticleSystemUtility::RunComputeShader(m_DeviceContext, m_InitComputeShader, 1, m_CpSRV.GetAddressOf(), m_CpUAV.Get(), m_ParticleNum, 1, 1);

	m_CpGetBuf = ParticleSystemUtility::CreateAndCopyToBuffer(m_Device, m_DeviceContext, m_CpResult.Get());

	m_DeviceContext->Map(m_CpGetBuf.Get(), 0, D3D11_MAP_READ, 0, &m_MappedSubResource);
	m_DeviceContext->Unmap(m_CpGetBuf.Get(), 0);
	////-------------------------------------------------

	m_SystemLifeTime = m_ParticleState.m_DuaringTime + m_ParticleState.m_StartDelayTime;
	isSystemActive = true;
	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
		m_ParticleVec.shrink_to_fit();
	}
}


//�`�惁�\�b�h
void ParticleSystem::DrawNomal(const XMFLOAT4X4& CameraMatrix) {

	m_BillBoard.SetDrawUtility();
	for (int ParticleNum = 0; ParticleNum < m_ParticleVec.size(); ParticleNum++) {
		if (m_ParticleVec[ParticleNum].isAlive == false) {
			continue;
		}

		//�`��
		m_BillBoard.SetPosition(m_ParticleVec[ParticleNum].Matrix._41, m_ParticleVec[ParticleNum].Matrix._42, m_ParticleVec[ParticleNum].Matrix._43);
		m_BillBoard.SetSize(m_ParticleState.m_Size, m_ParticleState.m_Size);
		m_BillBoard.SetColor(XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]));
		m_BillBoard.DrawOnly(CameraMatrix, (float)m_ParticleVec[ParticleNum].ZAngle);
	}
}

//GPU�p�[�e�B�N���p�̕`�惁�\�b�h
void ParticleSystem::GPUDraw(const XMFLOAT4X4& CameraMatrix) {

	m_BillBoard.SetDrawUtility();
	for (int Count = 0; Count < m_ParticleNum; Count++) {
		if (OutState[Count].isAlive == false || OutState[Count].isWaiting == 1) {
			continue;
		}

		//�`��
		m_BillBoard.SetPosition(OutState[Count].Matrix._41, OutState[Count].Matrix._42, OutState[Count].Matrix._43);
		m_BillBoard.SetSize(m_ParticleState.m_Size, m_ParticleState.m_Size);
		m_BillBoard.SetColor(XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]));
		m_BillBoard.DrawOnly(CameraMatrix, (float)OutState[Count].ZAngle);
	}
}

void ParticleSystem::UnInit() {
	//�I������
	if (Particles != nullptr) {
		delete[] Particles;
		Particles = nullptr;
	}

	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
		m_ParticleVec.shrink_to_fit();
	}
}

void ParticleSystem::AddParticle(m_Particles* AddParticle) {
	//�p�[�e�B�N�����X�g�ɒǉ�
	m_Particles newParticle;
	XMFLOAT4 qt;//�N�H�[�^�j�I��

	XMFLOAT4 axisX, axisY, axisZ;//�x�N�g���ۑ�
	XMFLOAT4 qtx, qty, qtz;//�N�H�[�^�j�I��
	XMFLOAT4 tempqt1, tempqt2, tempqt3;//�ꎞ�ۑ��N�H�[�^�j�I��

	DX11MatrixIdentity_subDefiner(newParticle.Matrix);

	//�p�x�����o��-------------------------------
	axisX.x = newParticle.Matrix._11;
	axisX.y = newParticle.Matrix._12;
	axisX.z = newParticle.Matrix._13;

	axisY.x = newParticle.Matrix._21;
	axisY.y = newParticle.Matrix._22;
	axisY.z = newParticle.Matrix._23;

	axisZ.x = newParticle.Matrix._31;
	axisZ.y = newParticle.Matrix._32;
	axisZ.z = newParticle.Matrix._33;
	//-------------------------------------------
		
	//�s�񂩂�N�H�[�^�j�I���𐶐�
	DX11GetQtfromMatrix(newParticle.Matrix, qt);

	//�����_���Ȋp�x�����ꂼ��ݒ�
	//�w�莲��]�̃N�H�[�^�j�I���𐶐�
	DX11QtRotationAxis(qtx, axisX, (float)(rand() % m_ParticleState.m_AngleRange) + m_ParticleState.m_Angle[0]);
	DX11QtRotationAxis(qty, axisY, (float)(rand() % m_ParticleState.m_AngleRange) + m_ParticleState.m_Angle[1]);
	DX11QtRotationAxis(qtz, axisZ, (float)(rand() % m_ParticleState.m_AngleRange) + m_ParticleState.m_Angle[2]);

	//�N�H�[�^�j�I���𐳋K��

	//�N�H�[�^�j�I��������

	DX11QtMul(tempqt1, qt, qtx);

	DX11QtMul(tempqt2, qty, qtz);

	DX11QtMul(tempqt3, tempqt1, tempqt2);

	//�N�H�[�^�j�I�����m�[�}���C�Y
	DX11QtNormalize(tempqt3, tempqt3);

	DX11MtxFromQt(newParticle.Matrix, tempqt3);

	newParticle.Matrix._41 = -1 * m_ParticleState.m_Position[0];
	newParticle.Matrix._42 = m_ParticleState.m_Position[1];
	newParticle.Matrix._43 = m_ParticleState.m_Position[2];

	//�����܂ł̑ҋ@���Ԑݒ�
	//�p�[�e�B�N���̔������Ԃ��p�[�e�B�N�����œ������A���ꂼ�ꊄ�蓖�Ă�
	newParticle.DelayTime = 0;
	//���̑��ݒ�
	newParticle.LifeTime = m_ParticleState.m_MaxLifeTime;

	newParticle.ZAngle = rand() % 360;
	
	newParticle.CountTime = 0;
	//�p�[�e�B�N����ǉ�
	m_ParticleVec.push_back(newParticle);

}
bool ParticleSystem::FInState(const char* FileName_) {
	//�p�[�e�B�N���V�X�e���̏����t�@�C������ǂݍ���

	//�J�����g�f�B���N�g���𑀍삷��
	TCHAR crDir[MAX_PATH + 1];
	t_ParticleSystemState GetState;


	GetCurrentDirectory(MAX_PATH + 1, crDir);//����O�̃f�B���N�g�����擾
	SetCurrentDirectory(".\\InPutData");//�ǂݍ��ݐ�t�H���_�փJ�����g�f�B���N�g����ύX

	//�t�@�C���ǂݍ���
	FILE *Fp;
	errno_t ErrorCheck;
	try {
		ErrorCheck = fopen_s(&Fp, FileName_, "rb");//

		SetCurrentDirectory(crDir);//�J�����g�f�B���N�g�������ɖ߂�

		if (ErrorCheck != 0) {
			throw 1;
		}
		fread(&GetState, sizeof(t_ParticleSystemState), 1, Fp);//�f�[�^�ǂݍ���
		
		SetParticleSystemState(&GetState);//���Z�b�g

		fclose(Fp);
	}
	catch (int i) {//�t�@�C���ǂݍ��ݎ��s������
		return i;
	};

	return true;
}

bool ParticleSystem::FInTex(const char* FileName_) {
	//�e�N�X�`���ǂݍ���
	std::string Texname = ".\\InPutData/";
	Texname += FileName_;//�t�@�C���̈ʒu���w��

	Init(m_Device, m_DeviceContext,&m_ParticleState, Texname.c_str());//�w�肵���t�@�C���𗘗p���ď�����
	(this->*fpStartFunc)();
	return 0;
}

void ParticleSystem::FOutState() {
	//�p�[�e�B�N���V�X�e���̏����t�@�C���֏����o��

	//�J�����g�f�B���N�g���𑀍삷��
	TCHAR crDir[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH + 1, crDir);//����O�̃f�B���N�g�����擾
	SetCurrentDirectory(".\\OutPutData");//�����o����̃t�H���_�փJ�����g�f�B���N�g����ύX

	//�t�@�C�������o��
	FILE *Fp;

	fopen_s(&Fp, "ParticleState.txt", "wb");

	fwrite(&m_ParticleState, sizeof(t_ParticleSystemState), 1, Fp);

	fclose(Fp);

	SetCurrentDirectory(crDir);//�J�����g�f�B���N�g�������ɖ߂�
}

//�A�N�Z�T
void ParticleSystem::SetParticleSystemState(t_ParticleSystemState* SetParticleSystemState_) {
	//�p�[�e�B�N���V�X�e���̐ݒ�𔽉f������
	if (SetParticleSystemState_ == nullptr) {
		return;
	}
	memcpy(&m_ParticleState, SetParticleSystemState_, sizeof(t_ParticleSystemState));
	ChangeGPUParticleMode(SetParticleSystemState_->isGPUParticle);
}
//���O�ݒ�
void ParticleSystem::SetName(const char* setName) {
	strcpy_s(m_ParticleState.m_Name, setName);
}

ParticleSystem& ParticleSystem::SetActive(bool set) {
	m_ParticleState.isActive = set;
	return *this;
}

ParticleSystem& ParticleSystem::SetEmitte(bool set) {
	isEmitting = set;
	return *this;
}

//�p�[�e�B�N���I�����ɌĂяo���ʃp�[�e�B�N���V�X�e���ݒ�
void ParticleSystem::SetNextParticleSystem(int NextNumber) {
	//m_ParticleState.m_NextSystemNumber = NextNumber;
	if (NextNumber == -1) {
		m_NextParticleNumberVector.clear();
		m_NextParticleNumberVector.shrink_to_fit();
		return;
	}

	m_NextParticleNumberVector.emplace_back(NextNumber);
	//�\�[�g���ĘA�������d���v�f���폜
	std::sort(m_NextParticleNumberVector.begin(), m_NextParticleNumberVector.end());
	m_NextParticleNumberVector.erase(std::unique(m_NextParticleNumberVector.begin(), m_NextParticleNumberVector.end()), m_NextParticleNumberVector.end());
}

//���\�b�h�ł̏������̂��̗̂L����������
void ParticleSystem::SetisUpdateActive(bool active) {
	isUpdateActive = active;
}
void ParticleSystem::SetisDrawActive(bool active) {
	isDrawActive = active;
}

//�R���s���[�g�V�F�[�_�[�ݒ�
ParticleSystem& ParticleSystem::SetComputeShader(ID3D11ComputeShader* setShader, eComputeShaderType type) {
	switch (type)
	{
	case eComputeShaderType::INIT:
		m_InitComputeShader = setShader;
		break;

	case eComputeShaderType::UPDATE:
		m_ComputeShader = setShader;
		break;

	default:
		break;
	}
	return *this;
}

//�ڕW���W��ݒ�
void ParticleSystem::SetTargetPos(float x, float y, float z) {
	m_TargetPos.x = x;
	m_TargetPos.y = y;
	m_TargetPos.z = z;
}

//���ʔԍ���ݒ�
ParticleSystem& ParticleSystem::setSystemNumber(int setNumber) {
	m_ParticleState.m_SystemNumber = setNumber;
	return *this;
}

//�\�t�g�p�[�e�B�N���p�萔�o�b�t�@��ݒ�
void ParticleSystem::SetSoftPConstantBuffer(ConstantBufferSoftParticle* setState) {
	if (setState != nullptr) {
		memcpy_s(&m_ParticleState.m_CBSoftParticleState, sizeof(ConstantBufferSoftParticle), setState, sizeof(ConstantBufferSoftParticle));
	}
	
	m_DeviceContext->UpdateSubresource(m_CpCBufferSoftParticle.Get(), 0, nullptr, &m_ParticleState.m_CBSoftParticleState, 0, 0);

	m_DeviceContext->PSSetConstantBuffers(5, 1, m_CpCBufferSoftParticle.GetAddressOf());
}

//�r���[�|�[�g�p�萔�o�b�t�@��ݒ�
ParticleSystem& ParticleSystem::SetViewPort(UINT* viewport) {
	m_ParticleState.m_CBSoftParticleState.iViewPort[0] = viewport[0];
	m_ParticleState.m_CBSoftParticleState.iViewPort[1] = viewport[1];

	m_DeviceContext->UpdateSubresource(m_CpCBufferSoftParticle.Get(), 0, nullptr, &m_ParticleState.m_CBSoftParticleState, 0, 0);

	m_DeviceContext->PSSetConstantBuffers(5, 1, m_CpCBufferSoftParticle.GetAddressOf());

	return *this;
}

//getter
float* ParticleSystem::getMatrixf16() {
	//TODO ��]�ړ��̔��f
	float retMat[16] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		m_ParticleState.m_Position[0], m_ParticleState.m_Position[1], m_ParticleState.m_Position[2], 1.f
	};

	return retMat;
}

t_ParticleSystemState ParticleSystem::GetState() {
	return m_ParticleState;
}

char* ParticleSystem::getName() {
	return m_ParticleState.m_Name;
}

int ParticleSystem::getSystemNumber() {
	return m_ParticleState.m_SystemNumber;
}

int ParticleSystem::getNextSystemNumber() {
	return m_ParticleState.m_NextSystemNumber;
}

std::vector<int> ParticleSystem::getNextSystemNumbers() {
	return m_NextParticleNumberVector;
}

//���\�b�h�ł̏������̂��̗̂L����������
bool ParticleSystem::getisUpdateActive() const {
	return isUpdateActive;
}
bool ParticleSystem::getisDrawActive() const {
	return isDrawActive;
}


float ParticleSystem::getLifeTime() {
	return m_SystemLifeTime;
}

ConstantBufferSoftParticle ParticleSystem::getCBSoftParticleState() {
	return m_ParticleState.m_CBSoftParticleState;
}

void ParticleSystem::ChangeGPUParticleMode(bool isGPUMode) {
	//�p�[�e�B�N���̓��샂�[�h�ɂ���Ď��s���郁�\�b�h��؂�ւ���
	if (isGPUMode) {
		fpStartFunc = &ParticleSystem::StartGPUParticle;
		fpUpdateFunc = &ParticleSystem::UpdateComputeShader;
		fpDrawFunc = &ParticleSystem::GPUDraw;
	}
	else {
		fpStartFunc = &ParticleSystem::StartNomalParticle;
		fpUpdateFunc = &ParticleSystem::UpdateNomal;
		fpDrawFunc = &ParticleSystem::DrawNomal;
	}
}

void ParticleSystem::ChangeSoftParticleMode(bool isSoftParticle) {
	//�\�t�g�p�[�e�B�N�����ǂ����ɂ���ė��p����V�F�[�_�[��؂�ւ��A�r���{�[�h������������
	m_ParticleState.isSoftParticle = isSoftParticle;
	if (isSoftParticle) {
		m_BillBoard.Init(m_Device, m_DeviceContext, 0, 0, 0,
			m_ParticleState.m_Size, m_ParticleState.m_Size,
			XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]),
			SOFTPARTICLE_PS_SHADER,
			SOFTPARTICLE_VS_SHADER);
	}
	else {
		m_BillBoard.Init(m_Device, m_DeviceContext, 0, 0, 0,
			m_ParticleState.m_Size, m_ParticleState.m_Size,
			XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]),
			PARTICLE_PS_SHADER,
			PARTICLE_VS_SHADER);
	}
}