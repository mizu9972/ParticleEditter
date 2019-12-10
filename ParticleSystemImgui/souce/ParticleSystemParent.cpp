#include "ParticleSystemParent.h"

//��{����
void ParticleSystemParent::Init() {
	//������
	ID3D11Device* device = CDirectXGraphics::GetInstance()->GetDXDevice();
	//�R���s���[�g�V�F�[�_�[���쐬
	CreateComputeShader(device, "Shader/csParticle.hlsl", "main", "cs_5_0", &m_ComputeShader);
	CreateComputeShader(device, "Shader/csInitParticle.hlsl", "main", "cs_5_0", &m_InitComputeShader);
}
void ParticleSystemParent::UnInit() {
	//�I������
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
	//�p�[�e�B�N���Đ�
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

//�t�@�C������ǂݍ���
void ParticleSystemParent::InputParticleSystem(const char* filename) {
	DeleteParticleSystem();

	FILE *fp;
	t_ParticleSystemState GetState;

	fopen_s(&fp, filename, "rb");

	int ParticleCount = 0;
	fread(&ParticleCount, sizeof(int), 1, fp);//read1 //���p�[�e�B�N���V�X�e����
	
	std::vector<int> nextNumbers;
	int NumberSize;

	//read1�œǂݍ��񂾃p�[�e�B�N���V�X�e�������[�v
	for (int Count = 0; Count < ParticleCount; Count++) {
		//�f�[�^��ǂݍ���Œǉ�
		fread(&GetState, sizeof(t_ParticleSystemState), 1, fp);//read2 �p�[�e�B�N���̐ݒ�ǂݍ���
		
		fread(&NumberSize, sizeof(int), 1, fp);//read3 ���p�[�e�B�N�������擾
		
		nextNumbers.clear();
		for (int num = 0; num < NumberSize; num++) {
			int setNumber;
			fread(&setNumber, sizeof(int), 1, fp);//read4 �v�f�ǂݍ���
			nextNumbers.emplace_back(setNumber);
		}

		AddParticleSystem(&GetState, nextNumbers);//���f
	}
	setParticleCounter(ParticleCount);//����ǉ�����p�[�e�B�N���V�X�e���̔ԍ������Ȃ��悤�Ƀp�[�e�B�N���V�X�e�������ݒ�
	fclose(fp);
}

//���X�g���쏈��
ParticleSystem* ParticleSystemParent::AddParticleSystem() {
	//�ǉ�
	ParticleSystem* newParticleSystem = new ParticleSystem;

	newParticleSystem->
		SetComputeShader(m_InitComputeShader, ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader, ParticleSystem::eComputeShaderType::UPDATE)
		.Init()
		.setSystemNumber(m_ParticleCounter);
		
	newParticleSystem->AddObsever(this);
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}

//�p�[�e�B�N���V�X�e���ǉ�
ParticleSystem* ParticleSystemParent::AddParticleSystem(t_ParticleSystemState* setState) {
	//�ǉ�
	ParticleSystem* newParticleSystem = new ParticleSystem;

	newParticleSystem->
		SetComputeShader(m_InitComputeShader,ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader,ParticleSystem::eComputeShaderType::UPDATE)
		.SetEmitte(setState->isEmitting)
		.Init(setState)
		.setSystemNumber(m_ParticleCounter);
		
	newParticleSystem->AddObsever(this);
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}

ParticleSystem* ParticleSystemParent::AddParticleSystem(t_ParticleSystemState* setState, std::vector<int>& setNumbers) {
	//�ǉ�
	ParticleSystem* newParticleSystem = new ParticleSystem;

	newParticleSystem->
		SetComputeShader(m_InitComputeShader, ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader, ParticleSystem::eComputeShaderType::UPDATE)
		.SetEmitte(setState->isEmitting)
		.Init(setState)
		.setSystemNumber(m_ParticleCounter);
	
	for (int num = 0; num < setNumbers.size(); num++) {
		newParticleSystem->SetNextParticleSystem(setNumbers[num]);
	}

	newParticleSystem->AddObsever(this);
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}

//�p�[�e�B�N���V�X�e���폜
void ParticleSystemParent::RemoveParticleSystem(ParticleSystem* pParticleSystem_) {
	//�폜
	delete pParticleSystem_;
}

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

//�p�[�e�B�N���V�X�e���I���ʒm�󂯎��
void ParticleSystemParent::OnNotify(Subject* subject_) {
	ParticleSystem* PSystem = reinterpret_cast<ParticleSystem*>(subject_);
	std::vector<int> nextNumbers = PSystem->getNextSystemNumbers();

	//���ɋN������p�[�e�B�N���V�X�e�����p�[�e�B�N���V�X�e���Q����Ăяo���ċN��
	for (int num = 0; num < nextNumbers.size(); num++) {
		m_ParticleSystemDictionary[nextNumbers[num]]->SetActive(true).SetEmitte(false).Start();
	}
 //	int NextSystemNumber = PSystem->getNextSystemNumber();
	//m_ParticleSystemDictionary[NextSystemNumber]->SetActive(true)
	//	.Start();
}