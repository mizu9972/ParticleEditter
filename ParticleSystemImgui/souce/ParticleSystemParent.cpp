#include "ParticleSystemParent.h"

//基本処理
void ParticleSystemParent::Init() {
	//初期化
	ID3D11Device* device = CDirectXGraphics::GetInstance()->GetDXDevice();
	//コンピュートシェーダーを作成
	CreateComputeShader(device, "Shader/csParticle.hlsl", "main", "cs_5_0", &m_ComputeShader);
	CreateComputeShader(device, "Shader/csInitParticle.hlsl", "main", "cs_5_0", &m_InitComputeShader);
}
void ParticleSystemParent::UnInit() {
	//終了処理
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
	//パーティクル再生
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

//ファイルから読み込み
void ParticleSystemParent::InputParticleSystem(const char* filename) {
	DeleteParticleSystem();

	FILE *fp;
	t_ParticleSystemState GetState;

	fopen_s(&fp, filename, "rb");

	int ParticleCount = 0;
	fread(&ParticleCount, sizeof(int), 1, fp);//read1 //総パーティクルシステム数
	
	std::vector<int> nextNumbers;
	int NumberSize;

	//read1で読み込んだパーティクルシステム分ループ
	for (int Count = 0; Count < ParticleCount; Count++) {
		//データを読み込んで追加
		fread(&GetState, sizeof(t_ParticleSystemState), 1, fp);//read2 パーティクルの設定読み込み
		
		fread(&NumberSize, sizeof(int), 1, fp);//read3 他パーティクル数を取得
		
		nextNumbers.clear();
		for (int num = 0; num < NumberSize; num++) {
			int setNumber;
			fread(&setNumber, sizeof(int), 1, fp);//read4 要素読み込み
			nextNumbers.emplace_back(setNumber);
		}

		AddParticleSystem(&GetState, nextNumbers);//反映
	}
	setParticleCounter(ParticleCount);//今後追加するパーティクルシステムの番号が被らないようにパーティクルシステム総数設定
	fclose(fp);
}

//リスト操作処理
ParticleSystem* ParticleSystemParent::AddParticleSystem() {
	//追加
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

//パーティクルシステム追加
ParticleSystem* ParticleSystemParent::AddParticleSystem(t_ParticleSystemState* setState) {
	//追加
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
	//追加
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

//パーティクルシステム削除
void ParticleSystemParent::RemoveParticleSystem(ParticleSystem* pParticleSystem_) {
	//削除
	delete pParticleSystem_;
}

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

//パーティクルシステム終了通知受け取り
void ParticleSystemParent::OnNotify(Subject* subject_) {
	ParticleSystem* PSystem = reinterpret_cast<ParticleSystem*>(subject_);
	std::vector<int> nextNumbers = PSystem->getNextSystemNumbers();

	//次に起動するパーティクルシステムをパーティクルシステム群から呼び出して起動
	for (int num = 0; num < nextNumbers.size(); num++) {
		m_ParticleSystemDictionary[nextNumbers[num]]->SetActive(true).SetEmitte(false).Start();
	}
 //	int NextSystemNumber = PSystem->getNextSystemNumber();
	//m_ParticleSystemDictionary[NextSystemNumber]->SetActive(true)
	//	.Start();
}