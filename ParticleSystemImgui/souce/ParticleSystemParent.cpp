#include "ParticleSystemParent.h"

//��{����
void ParticleSystemParent::Init() {
	ID3D11Device* device = CDirectXGraphics::GetInstance()->GetDXDevice();
	//�R���s���[�g�V�F�[�_�[���쐬
	CreateComputeShader(device, "Shader/csParticle.hlsl", "main", "cs_5_0", &m_ComputeShader);
	CreateComputeShader(device, "Shader/csInitParticle.hlsl", "main", "cs_5_0", &m_InitComputeShader);
}
void ParticleSystemParent::UnInit() {

	for (auto iParticleSystem : m_ParticleSystemDictionary) {
		iParticleSystem.second->UnInit();
		iParticleSystem.second->RemoveObserver(this);
	}
	
	if (m_ComputeShader != nullptr) {
		m_ComputeShader->Release();
		m_ComputeShader = nullptr;
	}
}
void ParticleSystemParent::Start() {
	for (auto iParticleSystem : m_ParticleSystemDictionary) {
		iParticleSystem.second->Start();
	}
}
void ParticleSystemParent::Update() {
	//�X�V
	for (auto iParticleSystem : m_ParticleSystemDictionary) {
		iParticleSystem.second->Update();
	}

}
void ParticleSystemParent::Draw() {
	//�`��

	for (auto iParticleSystem : m_ParticleSystemDictionary) {
		iParticleSystem.second->Draw(GetDX11DeviceContext());
	}
}


//���X�g���쏈��
ParticleSystem* ParticleSystemParent::AddParticleSystem() {
	//�ǉ�
	ParticleSystem* newParticleSystem = new ParticleSystem;

	newParticleSystem->
		SetComputeShader(m_InitComputeShader, ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader, ParticleSystem::eComputeShaderType::UPDATE)
		.Init();
	newParticleSystem->setSystemNumber(m_ParticleCounter);
		
	newParticleSystem->AddObsever(this);
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}

ParticleSystem* ParticleSystemParent::AddParticleSystem(t_ParticleSystemState* setState) {
	//�ǉ�
	ParticleSystem* newParticleSystem = new ParticleSystem;

	newParticleSystem->
		SetComputeShader(m_InitComputeShader,ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader,ParticleSystem::eComputeShaderType::UPDATE)
		.Init(setState);
	newParticleSystem->setSystemNumber(m_ParticleCounter);
		
	newParticleSystem->AddObsever(this);
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}

void ParticleSystemParent::RemoveParticleSystem(ParticleSystem* pParticleSystem_) {
	//�폜
	delete pParticleSystem_;
}

//�p�[�e�B�N���V�X�e���폜
void ParticleSystemParent::RemoveParticleSystem(int removeKey) {

	m_ParticleSystemDictionary[removeKey]->RemoveObserver(this);
	m_ParticleSystemDictionary.erase(removeKey);
}

void ParticleSystemParent::DeleteParticleSystem() {
	//�S�폜

	for (auto iParticleSystem : m_ParticleSystemDictionary) {
		iParticleSystem.second->UnInit();
		iParticleSystem.second->RemoveObserver(this);
	}
	m_ParticleCounter = 0;

	m_ParticleSystemDictionary.clear();
}

//�p�[�e�B�N���V�X�e���I���ʒm
void ParticleSystemParent::OnNotify(Subject* subject_) {
	ParticleSystem* PSystem = reinterpret_cast<ParticleSystem*>(subject_);
 	int NextSystemNumber = PSystem->getNextSystemNumber();
	m_ParticleSystemDictionary[NextSystemNumber]->SetActive(true)
		.Start();
}