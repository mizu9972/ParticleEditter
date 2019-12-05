//�p�[�e�B�N���V�X�e���v���O����

//�w�b�_�[
#include <random>
#include "CParticle.h"
#include "dx11mathutil.h"
#include "CCamera.h"
#include "CDirectxGraphics.h"

//�f�o�b�O�p
#include "CTimer.h"


#define		SCREEN_X		1200
#define		SCREEN_Y		600
#define		PARTICLE_NUM_PER_THREAD (64) //csParticle.hlsl���Ɠ����萔��錾
#define		THREAD_NUM (256)

#ifndef     ALIGN16
#define     ALIGN16 _declspec(align(16))
#endif

constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/particle.png";
constexpr auto PARTICLE_PS_SHADER = "shader/psParticle.fx";
constexpr auto PARTICLE_VS_SHADER = "shader/vsPrticle.fx";
/*------------------------
�P�ʍs��ɂ���
--------------------------*/
void DX11MatrixIdentity_subDefiner(DirectX::XMFLOAT4X4& mat) {

	ALIGN16 XMMATRIX mtx;

	mtx = XMMatrixIdentity();

	XMStoreFloat4x4(&mat, mtx);
}

//���\�b�h
//������
void ParticleSystem::Init() {

	//�p�[�e�B�N��������
	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
	}

	//�����X�e�[�^�X��ݒ�
	t_ParticleSystemState newState;
	SetParticleSystemState(&newState);
	
	InitComputeShader();
	
	//�r���{�[�h������---------------------------------------------------------------------------------------------------------------
	m_BillBoard.Init(0, 0, 0,
		m_ParticleState.m_Size, m_ParticleState.m_Size,
		XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]),
		PARTICLE_PS_SHADER,
		PARTICLE_VS_SHADER);
	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };

	m_BillBoard.SetUV(u, v);
	m_BillBoard.LoadTexTure(m_ParticleState.m_TextureName);
	//----------------------------------------------------------------------------------------------------------------------------


	Start();
}

void ParticleSystem::Init(t_ParticleSystemState* ParticleState_) {
	//�p�[�e�B�N��������
	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
	}

	//�����̃X�e�[�^�X��ݒ�
	SetParticleSystemState(ParticleState_);

	InitComputeShader();

	//�r���{�[�h������---------------------------------------------------------------------------------------------------------------
	m_BillBoard.Init(0, 0, 0,
		ParticleState_->m_Size, ParticleState_->m_Size,
		XMFLOAT4(ParticleState_->m_Color[0], ParticleState_->m_Color[1], ParticleState_->m_Color[2], ParticleState_->m_Color[3]),
		PARTICLE_PS_SHADER,
		PARTICLE_VS_SHADER);

	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
	m_BillBoard.SetUV(u, v);
	m_BillBoard.LoadTexTure(m_ParticleState.m_TextureName);
	//----------------------------------------------------------------------------------------------------------------------------
	Start();
}
void ParticleSystem::Init(t_ParticleSystemState ParticleState_, const char* filename, ID3D11Device* device) {
	//�p�[�e�B�N��������
	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
	}

	//�����̃X�e�[�^�X��ݒ�
	SetParticleSystemState(&ParticleState_);

	// �t�@�C������ۑ�
	strcpy_s(m_ParticleState.m_TextureName, filename);

	InitComputeShader();

	//�r���{�[�h������---------------------------------------------------------------------------------------------------------------
	m_BillBoard.Init(0, 0, 0,
		m_ParticleState.m_Size, m_ParticleState.m_Size, 
		XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]),
		PARTICLE_PS_SHADER,
		PARTICLE_VS_SHADER);

	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
	m_BillBoard.SetUV(u, v);
	m_BillBoard.LoadTexTure(m_ParticleState.m_TextureName);
	//----------------------------------------------------------------------------------------------------------------------------
	Start();
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
	ID3D11Device* device = CDirectXGraphics::GetInstance()->GetDXDevice();
	ID3D11DeviceContext* devicecontext = CDirectXGraphics::GetInstance()->GetImmediateContext();	

	m_CpSRV.Attach(m_pSRV);
	m_CpUAV.Attach(m_pUAV);
	m_CpResult.Attach(m_pResult);
	m_CpConstantBuffer.Attach(m_ConstantBuffer);
	m_CpGetBuf.Attach(getbuf);

	CreateConstantBuffer(CDirectXGraphics::GetInstance()->GetDXDevice(),
		sizeof(m_ConstantBufferParticle),
		&m_CpConstantBuffer);

}

void ParticleSystem::Update() {
	//�X�V����
	(this->*fpUpdateFunc)();
}

void ParticleSystem::UpdateComputeShader() {

	if (m_ParticleNum <= 0) {
		return;
	}

	ID3D11DeviceContext* devicecontext = CDirectXGraphics::GetInstance()->GetImmediateContext();

	//�R���s���[�g�V�F�[�_�[�����s
	const UINT dispatchX = UINT(ceil(float(m_ParticleNum) / float(THREAD_NUM * PARTICLE_NUM_PER_THREAD)));
	RunComputeShader(devicecontext, m_ComputeShader, 1, m_CpSRV.GetAddressOf(), m_CpUAV.Get(),dispatchX, 1, 1);

	//�f�[�^�󂯎��
	devicecontext->CopyResource(m_CpGetBuf.Get() , m_CpResult.Get());//�o�b�t�@�R�s�[
	devicecontext->Map(m_CpGetBuf.Get(), 0, D3D11_MAP_READ, 0, &m_MappedSubResource);

	OutState = reinterpret_cast<m_ParticleUAVState*>(m_MappedSubResource.pData);//�f�[�^�l��

	devicecontext->Unmap(m_CpGetBuf.Get(), 0);
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

		//�d�͌v�Z
		if (m_ParticleState.UseGravity) {
			m_ParticleVec[ParticleNum].Matrix._31 += m_ParticleState.m_Gravity[0] / 100.0f;
			m_ParticleVec[ParticleNum].Matrix._32 += m_ParticleState.m_Gravity[1] / 100.0f;
			m_ParticleVec[ParticleNum].Matrix._33 += m_ParticleState.m_Gravity[2] / 100.0f;
		}


		//���x���ړ�������
		m_ParticleVec[ParticleNum].Matrix._41 += m_ParticleVec[ParticleNum].Matrix._31 * Speed_ * NowTime;
		m_ParticleVec[ParticleNum].Matrix._42 += m_ParticleVec[ParticleNum].Matrix._32 * Speed_ * NowTime;
		m_ParticleVec[ParticleNum].Matrix._43 += m_ParticleVec[ParticleNum].Matrix._33 * Speed_ * NowTime;


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

		if (m_ParticleState.m_NextSystemNumber != -1) {
			//���̃p�[�e�B�N���J�n
			Notify(this);
		}

		if (m_ParticleState.isLooping == true) {//���[�v
			Start();
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
				p_PickParticle->isWaiting = false;
				p_PickParticle->isAlive = true;

				AddParticle(p_PickParticle);
			}
		}
	}
	
}

void ParticleSystem::UpdateConstantBuffer() {
	//�R���X�^���g�o�b�t�@�X�V-------------------------------------------------------------------------------------------------------
	{
		m_CbParticle.iPosition = { m_ParticleState.m_Position[0], m_ParticleState.m_Position[1],m_ParticleState.m_Position[2],0 };
		m_CbParticle.iAngle = { m_ParticleState.m_Angle[0],    m_ParticleState.m_Angle[1],   m_ParticleState.m_Angle[2],   0 };
		m_CbParticle.iAngleRange = m_ParticleState.m_AngleRange;
		m_CbParticle.iDuaringTime = m_ParticleState.m_DuaringTime;
		m_CbParticle.iMaxLifeTime = m_ParticleState.m_MaxLifeTime;
		m_CbParticle.iSpeed = m_ParticleState.m_Speed;
		m_CbParticle.iRotateSpeed = m_ParticleState.m_RotateSpeed;
		m_CbParticle.isActive = m_ParticleState.isActive;
		m_CbParticle.isLooping = m_ParticleState.isLooping;
		m_CbParticle.iParticleNum = m_ParticleState.m_ParticleNum;
		m_CbParticle.iTime = 1.0f / FPS;
		m_CbParticle.iTargetPosition = m_TargetPos;
		m_CbParticle.isChaser = m_ParticleState.isChaser;
		m_CbParticle.iMinChaseAngle = m_ParticleState.m_MinChaseAngle;
		m_CbParticle.iMaxChaseAngle = m_ParticleState.m_MaxChaseAngle;
		m_CbParticle.iGravity = { m_ParticleState.m_Gravity[0],m_ParticleState.m_Gravity[1],m_ParticleState.m_Gravity[2] };
		m_CbParticle.UseGravity = m_ParticleState.UseGravity;
	}
	CDirectXGraphics::GetInstance()->GetImmediateContext()->UpdateSubresource(
		m_CpConstantBuffer.Get(),
		0,
		NULL,
		&m_CbParticle,
		0,
		0
	);
	CDirectXGraphics::GetInstance()->GetImmediateContext()->CSSetConstantBuffers(
		7,
		1,
		m_CpConstantBuffer.GetAddressOf()
	);

	//--------------------------------------------------------------------------------------------------------------------------
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

//�p�[�e�B�N�������J�n
void ParticleSystem::Start() {
	
	//�p�[�e�B�N������
	if (m_ParticleState.isActive == false) {
		return;
	}
	if (Particles != NULL) {
		delete[] Particles;
	}
	Particles = new m_Particles[m_ParticleState.m_ParticleNum];
	isSystemActive = true;
	ParticlesDeathCount = 0;

	m_SystemLifeTime = m_ParticleState.m_DuaringTime;
	
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
		p_PickParticle->DelayTime = m_ParticleState.m_DuaringTime / m_MaxParticleNum * ParticlesNum;

		p_PickParticle->CountTime = 0;
		//�ҋ@����
		p_PickParticle->isAlive = false;
		p_PickParticle->isWaiting = true;
	}
	//--------------------------------------------------------------------------------------


}

void ParticleSystem::StartGPUParticle(){
	////GPU�p�[�e�B�N���ݒ�---------------------------------

	ID3D11Device* device = CDirectXGraphics::GetInstance()->GetDXDevice();
	ID3D11DeviceContext* devicecontext = CDirectXGraphics::GetInstance()->GetImmediateContext();
	m_ParticleNum = m_ParticleState.m_ParticleNum;
	if (m_CpUAV != nullptr) {
		m_CpUAV->Release();
		m_CpUAV = nullptr;
	}
	if (m_CpResult != nullptr) {
		m_CpResult->Release();
		m_CpResult = nullptr;
	}
	////���͗p�o�b�t�@���X�V
	//CreateStructuredBuffer(device, sizeof(m_ParticleSRVState), m_ParticleState.m_ParticleNum, nullptr, &m_pBuf);
	//CreateShaderResourceView(device, m_pBuf, &m_pSRV);
	//�o�͗p�o�b�t�@���X�V
	CreateStructuredBuffer(device, sizeof(m_ParticleUAVState), m_ParticleNum, nullptr, m_CpResult.GetAddressOf());
	CreateUnOrderAccessView(device, m_CpResult.Get(), m_CpUAV.GetAddressOf());

	m_CpGetBuf = CreateAndCopyToBuffer(device, devicecontext, m_CpResult.Get());
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	devicecontext->Map(m_CpGetBuf.Get(), 0, D3D11_MAP_READ, 0, &MappedSubResource);

	devicecontext->Unmap(m_CpGetBuf.Get(), 0);
	////-------------------------------------------------

	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
		m_ParticleVec.shrink_to_fit();
	}
}


//�`�揈��
void ParticleSystem::Draw(ID3D11DeviceContext* device) {

	(this->*fpDrawFunc)(device);
}

//�`�惁�\�b�h
void ParticleSystem::DrawNomal(ID3D11DeviceContext* device) {

	m_BillBoard.SetDrawUtility();
	for (int ParticleNum = 0; ParticleNum < m_ParticleVec.size(); ParticleNum++) {
		if (m_ParticleVec[ParticleNum].isAlive == false) {
			continue;
		}

		//�`��
		m_BillBoard.SetPosition(m_ParticleVec[ParticleNum].Matrix._41, m_ParticleVec[ParticleNum].Matrix._42, m_ParticleVec[ParticleNum].Matrix._43);
		m_BillBoard.SetSize(m_ParticleState.m_Size, m_ParticleState.m_Size);
		m_BillBoard.SetColor(XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]));
		m_BillBoard.DrawOnly(CCamera::GetInstance()->GetCameraMatrix(), (float)m_ParticleVec[ParticleNum].ZAngle);
	}
}

//GPU�p�[�e�B�N���p�̕`�惁�\�b�h
void ParticleSystem::GPUDraw(ID3D11DeviceContext* device) {

	m_BillBoard.SetDrawUtility();
	for (int Count = 0; Count < m_ParticleNum; Count++) {
		if (OutState[Count].isAlive == false || OutState[Count].isWaiting == 1) {
			continue;
		}

		//�`��
		m_BillBoard.SetPosition(OutState[Count].Matrix._41, OutState[Count].Matrix._42, OutState[Count].Matrix._43);
		m_BillBoard.SetSize(m_ParticleState.m_Size, m_ParticleState.m_Size);
		m_BillBoard.SetColor(XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]));
		m_BillBoard.DrawOnly(CCamera::GetInstance()->GetCameraMatrix(), (float)OutState[Count].ZAngle);
	}
}

void ParticleSystem::UnInit() {

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


	newParticle.Matrix._41 = m_ParticleState.m_Position[0];
	newParticle.Matrix._42 = m_ParticleState.m_Position[1];
	newParticle.Matrix._43 = m_ParticleState.m_Position[2];



	//�����܂ł̑ҋ@���Ԑݒ�
	//�p�[�e�B�N���̔������Ԃ��p�[�e�B�N�����œ������A���ꂼ�ꊄ�蓖�Ă�
	newParticle.DelayTime = 0;
	//���̑��ݒ�
	newParticle.LifeTime = m_ParticleState.m_MaxLifeTime;

	newParticle.ZAngle = rand() % 360;
	
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
/*
	Init(m_ParticleState, "assets/ParticleTexture/particle.png");*/
	return true;
}

bool ParticleSystem::FInTex(const char* FileName_) {
	//�e�N�X�`���ǂݍ���
	std::string Texname = ".\\InPutData/";
	Texname += FileName_;//�t�@�C���̈ʒu���w��

	Init(m_ParticleState, Texname.c_str(),CDirectXGraphics::GetInstance()->GetDXDevice());//�w�肵���t�@�C���𗘗p���ď�����

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

//�A�N�Z�X
void ParticleSystem::SetParticleSystemState(t_ParticleSystemState* SetParticleSystemState_) {
	memcpy(&m_ParticleState, SetParticleSystemState_, sizeof(t_ParticleSystemState));
}
void ParticleSystem::SetName(const char* setName) {
	strcpy_s(m_ParticleState.m_Name, setName);
}

ParticleSystem& ParticleSystem::SetActive(bool set) {
	m_ParticleState.isActive = set;
	return *this;
}

void ParticleSystem::SetNextParticleSystem(ParticleSystem* next) {
	//m_ParticleState.m_NextParticleSystem = next;
}

void ParticleSystem::SetNextParticleSystem(int NextNumber) {
	m_ParticleState.m_NextSystemNumber = NextNumber;
}

ParticleSystem& ParticleSystem::SetComputeShader(ID3D11ComputeShader* setShader) {
	m_ComputeShader = setShader;
	return *this;
}

void ParticleSystem::SetTargetPos(float x, float y, float z) {
	m_TargetPos.x = x;
	m_TargetPos.y = y;
	m_TargetPos.z = z;
}

ParticleSystem& ParticleSystem::setSystemNumber(int setNumber) {
	m_ParticleState.m_SystemNumber = setNumber;
	return *this;
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

void ParticleSystem::ChangeGPUParticleMode(bool isGPUMode) {
	//�p�[�e�B�N���̓��샂�[�h�ɂ���Ď��s���郁�\�b�h��؂�ւ���
	if (isGPUMode) {
		fpUpdateFunc = &ParticleSystem::UpdateComputeShader;
		fpDrawFunc = &ParticleSystem::GPUDraw;
	}
	else {
		fpUpdateFunc = &ParticleSystem::UpdateNomal;
		fpDrawFunc = &ParticleSystem::DrawNomal;
	}
}