#include "ParticleSystemParent.h"

//基本処理
void ParticleSystemParent::Init() {
	ID3D11Device* device = CDirectXGraphics::GetInstance()->GetDXDevice();
	//コンピュートシェーダーを作成
	CreateComputeShader(device, "Shader/csParticle.hlsl", "main", "cs_5_0", &m_ComputeShader);
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
	//更新
	for (auto iParticleSystem : m_ParticleSystemDictionary) {
		iParticleSystem.second->Update();
	}

}
void ParticleSystemParent::Draw() {
	//描画

	for (auto iParticleSystem : m_ParticleSystemDictionary) {
		iParticleSystem.second->Draw(GetDX11DeviceContext());
	}
}


//リスト操作処理
ParticleSystem* ParticleSystemParent::AddParticleSystem() {
	//追加
	ParticleSystem* newParticleSystem = new ParticleSystem;

	newParticleSystem->SetComputeShader(m_ComputeShader)
		.Init();
	newParticleSystem->setSystemNumber(m_ParticleCounter);
		
	newParticleSystem->AddObsever(this);
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}

ParticleSystem* ParticleSystemParent::AddParticleSystem(t_ParticleSystemState* setState) {
	//追加
	ParticleSystem* newParticleSystem = new ParticleSystem;

	newParticleSystem->SetComputeShader(m_ComputeShader)
		.Init(setState);
	newParticleSystem->setSystemNumber(m_ParticleCounter);
		
	newParticleSystem->AddObsever(this);
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}

void ParticleSystemParent::RemoveParticleSystem(ParticleSystem* pParticleSystem_) {
	//削除
	delete pParticleSystem_;
}

//パーティクルシステム削除
void ParticleSystemParent::RemoveParticleSystem(int removeKey) {

	m_ParticleSystemDictionary[removeKey]->RemoveObserver(this);
	m_ParticleSystemDictionary.erase(removeKey);
}

void ParticleSystemParent::DeleteParticleSystem() {
	//全削除

	for (auto iParticleSystem : m_ParticleSystemDictionary) {
		iParticleSystem.second->UnInit();
		iParticleSystem.second->RemoveObserver(this);
	}
	m_ParticleCounter = 0;

	m_ParticleSystemDictionary.clear();
}

void ParticleSystemParent::OnNotify(Subject* subject_) {
	ParticleSystem* PSystem = reinterpret_cast<ParticleSystem*>(subject_);
 	int NextSystemNumber = PSystem->getNextSystemNumber();
	m_ParticleSystemDictionary[NextSystemNumber]->SetActive(true)
		.Start();
}