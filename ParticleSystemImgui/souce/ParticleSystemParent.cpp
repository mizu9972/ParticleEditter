#include "ParticleSystemParent.h"

//基本処理
void ParticleSystemParent::Init(ID3D11Device* device, ID3D11DeviceContext* devicecontext, ID3D11DepthStencilView* depthstencilView, ID3D11RenderTargetView* RenderTargetView, IDXGISwapChain* SwapChain) {
	//初期化
	m_Device = device;
	m_Devicecontext = devicecontext;
	m_SwapChain = SwapChain;

	if (SwapChain != nullptr) {
		//専用のレンダーターゲットビュー作成(ソフトパーティクル処理で必要)
		CreateRTV(&m_RenderTargetView, DXGI_FORMAT_R32_FLOAT);
	}
	m_BackRTV = RenderTargetView;
	m_DepthstencilView = depthstencilView;

	//コンピュートシェーダーを作成
	CreateComputeShader(m_Device, "Shader/csParticle.hlsl", "main", "cs_5_0", &m_ComputeShader);
	CreateComputeShader(m_Device, "Shader/csInitParticle.hlsl", "main", "cs_5_0", &m_InitComputeShader);
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
	float ClearColor[4] = { 1.0f,1.0f,1.0f,1.0f };
	//描画
	if (m_RenderTargetView != nullptr && m_DepthstencilView != nullptr && m_BackRTV != nullptr) {
		//レンダーターゲットサーフェイスを設定
		ID3D11RenderTargetView* RTV[2] = { NULL,NULL };
		RTV[0] = m_BackRTV;
		RTV[1] = NULL;

		m_Devicecontext->OMSetRenderTargets(2, RTV, nullptr);
		//レンダーターゲットビューからシェーダーリソースビュー取得
		ID3D11ShaderResourceView* SRV = getSRVfromRTV(m_RenderTargetView);
		ID3D11ShaderResourceView* ResetSRV = nullptr;

		m_Devicecontext->PSSetShaderResources(1, 1, &SRV);//シェーダーリソースビュー設定
		//パーティクル描画
		for (auto iParticleSystem : m_ParticleSystemDictionary) {
			iParticleSystem.second->Draw();//シェーダーリソースビューを利用して描画
		}

		m_Devicecontext->PSSetShaderResources(1, 1, &ResetSRV);//シェーダーリソースビュー無効化

		if (SRV) {
			SRV->Release();
			SRV = nullptr;
		}
		//レンダーターゲットサーフェイスを元に戻す
		m_Devicecontext->ClearRenderTargetView(m_RenderTargetView, ClearColor);
		RTV[0] = m_BackRTV;
		RTV[1] = m_RenderTargetView;
		m_Devicecontext->OMSetRenderTargets(2, RTV, m_DepthstencilView);
		
	}
	else {
		//専用のレンダーターゲットビューを利用しない描画
		TurnOffZbuffer();//Zバッファオフ
		for (auto iParticleSystem : m_ParticleSystemDictionary) {
			iParticleSystem.second->Draw();
		}
		TurnOnZbuffer();//Zバッファオン
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
	//パーティクルシステム追加
	//初期設定で追加用

	ParticleSystem* newParticleSystem = new ParticleSystem;	
	
	//スワップチェーンの情報取得
	DXGI_SWAP_CHAIN_DESC ScDesc;
	m_SwapChain->GetDesc(&ScDesc);
	UINT Viewport[2] = { ScDesc.BufferDesc.Width,ScDesc.BufferDesc.Height };

	//初期化
	newParticleSystem->
		SetComputeShader(m_InitComputeShader, ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader, ParticleSystem::eComputeShaderType::UPDATE)
		.Init(m_Device, m_Devicecontext)
		.SetViewPort(Viewport)
		.setSystemNumber(m_ParticleCounter);
		
	newParticleSystem->AddObsever(this);//パーティクルシステム終了通知対象へ追加

	//辞書登録
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}
ParticleSystem* ParticleSystemParent::AddParticleSystem(t_ParticleSystemState* setState) {
	//パーティクルシステム追加
	//コピー等編集された設定で初期化用

	ParticleSystem* newParticleSystem = new ParticleSystem;

	//スワップチェーンの情報取得
	DXGI_SWAP_CHAIN_DESC ScDesc;
	m_SwapChain->GetDesc(&ScDesc);
	UINT Viewport[2] = { ScDesc.BufferDesc.Height,ScDesc.BufferDesc.Width };

	//初期化
	newParticleSystem->
		SetComputeShader(m_InitComputeShader, ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader, ParticleSystem::eComputeShaderType::UPDATE)
		.SetEmitte(setState->isEmitting)
		.Init(m_Device, m_Devicecontext)
		.SetViewPort(Viewport)
		.setSystemNumber(m_ParticleCounter);
		
	newParticleSystem->AddObsever(this);//パーティクルシステム終了通知対象へ追加

	//辞書登録
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}
ParticleSystem* ParticleSystemParent::AddParticleSystem(t_ParticleSystemState* setState, std::vector<int>& setNumbers) {
	//パーティクルシステム追加
	//ファイル読み込み用

	ParticleSystem* newParticleSystem = new ParticleSystem;

	//スワップチェーンの情報取得
	DXGI_SWAP_CHAIN_DESC ScDesc;
	m_SwapChain->GetDesc(&ScDesc);
	UINT Viewport[2] = { ScDesc.BufferDesc.Height,ScDesc.BufferDesc.Width };

	//初期化
	newParticleSystem->
		SetComputeShader(m_InitComputeShader, ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader, ParticleSystem::eComputeShaderType::UPDATE)
		.SetEmitte(setState->isEmitting)
		.Init(m_Device, m_Devicecontext)
		.SetViewPort(Viewport);
		//.setSystemNumber(m_ParticleCounter);
	
	//システム番号設定
	for (int num = 0; num < setNumbers.size(); num++) {
		newParticleSystem->SetNextParticleSystem(setNumbers[num]);
	}

	newParticleSystem->AddObsever(this);//パーティクルシステム終了通知対象へ追加

	//辞書登録
	m_ParticleSystemDictionary[newParticleSystem->getSystemNumber()] = newParticleSystem;
	m_ParticleCounter = max(newParticleSystem->getSystemNumber(), m_ParticleCounter + 1);

	return newParticleSystem;
}

//パーティクルシステム削除
void ParticleSystemParent::RemoveParticleSystem(int removeKey) {

	m_ParticleSystemDictionary[removeKey]->RemoveObserver(this);//通知対象から外す
	m_ParticleSystemDictionary.erase(removeKey);//削除
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

void ParticleSystemParent::TurnOnZbuffer() {
	if (m_RenderTargetView == nullptr || m_DepthstencilView == nullptr) {
		return;
	}
	m_Devicecontext->OMSetRenderTargets(
		1,
		&m_RenderTargetView,
		m_DepthstencilView
	);
}

void ParticleSystemParent::TurnOffZbuffer() {
	if (m_RenderTargetView == nullptr || m_DepthstencilView == nullptr) {
		return;
	}
	m_Devicecontext->OMSetRenderTargets(
		1,
		&m_RenderTargetView,
		nullptr
	);
}

//レンダーターゲットビューからシェーダーリソースビューを取得
ID3D11ShaderResourceView* ParticleSystemParent::getSRVfromRTV(ID3D11RenderTargetView* ResourceView) {
	ID3D11Resource* Resource = nullptr;
	ID3D11ShaderResourceView* outSRV = nullptr;

	if (ResourceView == nullptr) {
		//取得データ無し
		return nullptr;
	}

	ResourceView->GetResource(&Resource);
	m_Device->CreateShaderResourceView(Resource, nullptr, &outSRV);//SRV作成
	
	if (Resource) {
		Resource->Release();
		Resource = nullptr;
	}
	return outSRV;
}

//レンダーターゲットビュー作成
HRESULT ParticleSystemParent::CreateRTV(ID3D11RenderTargetView** outRTV, DXGI_FORMAT format) {
	HRESULT hr = E_FAIL;

	ID3D11Texture2D* Texture2D = nullptr;

	//スワップチェーンの情報取得
	DXGI_SWAP_CHAIN_DESC ScDesc;
	m_SwapChain->GetDesc(&ScDesc);

	UINT Width, Height;
	Width = ScDesc.BufferDesc.Width;
	Height = ScDesc.BufferDesc.Height;

	//Zバッファ用のテクスチャ作成
	D3D11_TEXTURE2D_DESC Texture2DDesc;
	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;

	ZeroMemory(&Texture2DDesc, sizeof(D3D11_TEXTURE2D_DESC)); 
	Texture2DDesc.ArraySize          = 1;
	Texture2DDesc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	Texture2DDesc.Usage              = D3D11_USAGE_DEFAULT;
	Texture2DDesc.Format             = format;
	Texture2DDesc.Width              = Width;
	Texture2DDesc.Height             = Height;
	Texture2DDesc.MipLevels          = 1;
	Texture2DDesc.SampleDesc.Count   = 1;
	Texture2DDesc.SampleDesc.Quality = 0;

	ZeroMemory(&RTVDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	RTVDesc.Format = Texture2DDesc.Format;
	if (Texture2DDesc.SampleDesc.Count == 1) {
		RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	}
	else {
		RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	}
	RTVDesc.Texture2D.MipSlice = 0;
	Texture2DDesc.MiscFlags = 0;//D3D11_RESOURCE_MISC_GENERATE_MIPS;

	hr = m_Device->CreateTexture2D(&Texture2DDesc, nullptr, &Texture2D);
	if (FAILED(hr)) {
		return hr;
	}
	
	hr = m_Device->CreateRenderTargetView(Texture2D, &RTVDesc, outRTV);
	if (FAILED(hr)) {
		return hr;
	}

	if (Texture2D) {
		Texture2D->Release();
		Texture2D = nullptr;
	}

	////深度マップテクスチャをレンダーターゲットにする際のデプスステンシルビュー用のテクスチャーを作成
	//D3D11_TEXTURE2D_DESC DepthDesc;
	//DepthDesc.Width = Width;
	//DepthDesc.Height = Height;
	//DepthDesc.MipLevels = 1;
	//DepthDesc.ArraySize = 1;
	//DepthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	//DepthDesc.SampleDesc.Count = 1;
	//DepthDesc.SampleDesc.Quality = 0;
	//DepthDesc.Usage = D3D11_USAGE_DEFAULT;
	//DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//DepthDesc.CPUAccessFlags = 0;
	//DepthDesc.MiscFlags = 0;

	//hr = m_Device->CreateTexture2D(&DepthDesc, nullptr, &Texture2D);
	//if (FAILED(hr)) {
	//	return hr;
	//}

	//hr = m_Device->CreateDepthStencilView(Texture2D, nullptr, &m_DepthstencilViewRTV);
	//if (FAILED(hr)) {
	//	return hr;
	//}


	//if (Texture2D) {
	//	Texture2D->Release();
	//	Texture2D = nullptr;
	//}

	return S_OK;
}
//パーティクルシステム終了通知受け取り
void ParticleSystemParent::OnNotify(Subject* subject_) {
	ParticleSystem* PSystem = reinterpret_cast<ParticleSystem*>(subject_);
	std::vector<int> nextNumbers = PSystem->getNextSystemNumbers();

	//次に起動するパーティクルシステムをパーティクルシステム群から呼び出して起動
	for (int num = 0; num < nextNumbers.size(); num++) {
		m_ParticleSystemDictionary[nextNumbers[num]]->SetActive(true).SetEmitte(false).Start();
	}
}