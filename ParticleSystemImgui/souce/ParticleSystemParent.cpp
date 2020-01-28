#include "ParticleSystemParent.h"

//��{����
void ParticleSystemParent::Init(ID3D11Device* device, ID3D11DeviceContext* devicecontext, ID3D11DepthStencilView* depthstencilView, ID3D11RenderTargetView* RenderTargetView, IDXGISwapChain* SwapChain) {
	//������
	m_Device = device;
	m_Devicecontext = devicecontext;
	m_SwapChain = SwapChain;

	if (SwapChain != nullptr) {
		//��p�̃����_�[�^�[�Q�b�g�r���[�쐬(�\�t�g�p�[�e�B�N�������ŕK�v)
		CreateRTV(&m_RenderTargetView, DXGI_FORMAT_R32_FLOAT);
	}
	m_BackRTV = RenderTargetView;
	m_DepthstencilView = depthstencilView;

	//�R���s���[�g�V�F�[�_�[���쐬
	CreateComputeShader(m_Device, "Shader/csParticle.hlsl", "main", "cs_5_0", &m_ComputeShader);
	CreateComputeShader(m_Device, "Shader/csInitParticle.hlsl", "main", "cs_5_0", &m_InitComputeShader);
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
	float ClearColor[4] = { 1.0f,1.0f,1.0f,1.0f };
	//�`��
	if (m_RenderTargetView != nullptr && m_DepthstencilView != nullptr && m_BackRTV != nullptr) {
		//�����_�[�^�[�Q�b�g�T�[�t�F�C�X��ݒ�
		ID3D11RenderTargetView* RTV[2] = { NULL,NULL };
		RTV[0] = m_BackRTV;
		RTV[1] = NULL;

		m_Devicecontext->OMSetRenderTargets(2, RTV, nullptr);
		//�����_�[�^�[�Q�b�g�r���[����V�F�[�_�[���\�[�X�r���[�擾
		ID3D11ShaderResourceView* SRV = getSRVfromRTV(m_RenderTargetView);
		ID3D11ShaderResourceView* ResetSRV = nullptr;

		m_Devicecontext->PSSetShaderResources(1, 1, &SRV);//�V�F�[�_�[���\�[�X�r���[�ݒ�
		//�p�[�e�B�N���`��
		for (auto iParticleSystem : m_ParticleSystemDictionary) {
			iParticleSystem.second->Draw();//�V�F�[�_�[���\�[�X�r���[�𗘗p���ĕ`��
		}

		m_Devicecontext->PSSetShaderResources(1, 1, &ResetSRV);//�V�F�[�_�[���\�[�X�r���[������

		if (SRV) {
			SRV->Release();
			SRV = nullptr;
		}
		//�����_�[�^�[�Q�b�g�T�[�t�F�C�X�����ɖ߂�
		m_Devicecontext->ClearRenderTargetView(m_RenderTargetView, ClearColor);
		RTV[0] = m_BackRTV;
		RTV[1] = m_RenderTargetView;
		m_Devicecontext->OMSetRenderTargets(2, RTV, m_DepthstencilView);
		
	}
	else {
		//��p�̃����_�[�^�[�Q�b�g�r���[�𗘗p���Ȃ��`��
		TurnOffZbuffer();//Z�o�b�t�@�I�t
		for (auto iParticleSystem : m_ParticleSystemDictionary) {
			iParticleSystem.second->Draw();
		}
		TurnOnZbuffer();//Z�o�b�t�@�I��
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
	//�p�[�e�B�N���V�X�e���ǉ�
	//�����ݒ�Œǉ��p

	ParticleSystem* newParticleSystem = new ParticleSystem;	
	
	//�X���b�v�`�F�[���̏��擾
	DXGI_SWAP_CHAIN_DESC ScDesc;
	m_SwapChain->GetDesc(&ScDesc);
	UINT Viewport[2] = { ScDesc.BufferDesc.Width,ScDesc.BufferDesc.Height };

	//������
	newParticleSystem->
		SetComputeShader(m_InitComputeShader, ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader, ParticleSystem::eComputeShaderType::UPDATE)
		.Init(m_Device, m_Devicecontext)
		.SetViewPort(Viewport)
		.setSystemNumber(m_ParticleCounter);
		
	newParticleSystem->AddObsever(this);//�p�[�e�B�N���V�X�e���I���ʒm�Ώۂ֒ǉ�

	//�����o�^
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}
ParticleSystem* ParticleSystemParent::AddParticleSystem(t_ParticleSystemState* setState) {
	//�p�[�e�B�N���V�X�e���ǉ�
	//�R�s�[���ҏW���ꂽ�ݒ�ŏ������p

	ParticleSystem* newParticleSystem = new ParticleSystem;

	//�X���b�v�`�F�[���̏��擾
	DXGI_SWAP_CHAIN_DESC ScDesc;
	m_SwapChain->GetDesc(&ScDesc);
	UINT Viewport[2] = { ScDesc.BufferDesc.Height,ScDesc.BufferDesc.Width };

	//������
	newParticleSystem->
		SetComputeShader(m_InitComputeShader, ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader, ParticleSystem::eComputeShaderType::UPDATE)
		.SetEmitte(setState->isEmitting)
		.Init(m_Device, m_Devicecontext)
		.SetViewPort(Viewport)
		.setSystemNumber(m_ParticleCounter);
		
	newParticleSystem->AddObsever(this);//�p�[�e�B�N���V�X�e���I���ʒm�Ώۂ֒ǉ�

	//�����o�^
	m_ParticleSystemDictionary[m_ParticleCounter] = newParticleSystem;
	m_ParticleCounter++;

	return newParticleSystem;
}
ParticleSystem* ParticleSystemParent::AddParticleSystem(t_ParticleSystemState* setState, std::vector<int>& setNumbers) {
	//�p�[�e�B�N���V�X�e���ǉ�
	//�t�@�C���ǂݍ��ݗp

	ParticleSystem* newParticleSystem = new ParticleSystem;

	//�X���b�v�`�F�[���̏��擾
	DXGI_SWAP_CHAIN_DESC ScDesc;
	m_SwapChain->GetDesc(&ScDesc);
	UINT Viewport[2] = { ScDesc.BufferDesc.Height,ScDesc.BufferDesc.Width };

	//������
	newParticleSystem->
		SetComputeShader(m_InitComputeShader, ParticleSystem::eComputeShaderType::INIT)
		.SetComputeShader(m_ComputeShader, ParticleSystem::eComputeShaderType::UPDATE)
		.SetEmitte(setState->isEmitting)
		.Init(m_Device, m_Devicecontext)
		.SetViewPort(Viewport);
		//.setSystemNumber(m_ParticleCounter);
	
	//�V�X�e���ԍ��ݒ�
	for (int num = 0; num < setNumbers.size(); num++) {
		newParticleSystem->SetNextParticleSystem(setNumbers[num]);
	}

	newParticleSystem->AddObsever(this);//�p�[�e�B�N���V�X�e���I���ʒm�Ώۂ֒ǉ�

	//�����o�^
	m_ParticleSystemDictionary[newParticleSystem->getSystemNumber()] = newParticleSystem;
	m_ParticleCounter = max(newParticleSystem->getSystemNumber(), m_ParticleCounter + 1);

	return newParticleSystem;
}

//�p�[�e�B�N���V�X�e���폜
void ParticleSystemParent::RemoveParticleSystem(int removeKey) {

	m_ParticleSystemDictionary[removeKey]->RemoveObserver(this);//�ʒm�Ώۂ���O��
	m_ParticleSystemDictionary.erase(removeKey);//�폜
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

//�����_�[�^�[�Q�b�g�r���[����V�F�[�_�[���\�[�X�r���[���擾
ID3D11ShaderResourceView* ParticleSystemParent::getSRVfromRTV(ID3D11RenderTargetView* ResourceView) {
	ID3D11Resource* Resource = nullptr;
	ID3D11ShaderResourceView* outSRV = nullptr;

	if (ResourceView == nullptr) {
		//�擾�f�[�^����
		return nullptr;
	}

	ResourceView->GetResource(&Resource);
	m_Device->CreateShaderResourceView(Resource, nullptr, &outSRV);//SRV�쐬
	
	if (Resource) {
		Resource->Release();
		Resource = nullptr;
	}
	return outSRV;
}

//�����_�[�^�[�Q�b�g�r���[�쐬
HRESULT ParticleSystemParent::CreateRTV(ID3D11RenderTargetView** outRTV, DXGI_FORMAT format) {
	HRESULT hr = E_FAIL;

	ID3D11Texture2D* Texture2D = nullptr;

	//�X���b�v�`�F�[���̏��擾
	DXGI_SWAP_CHAIN_DESC ScDesc;
	m_SwapChain->GetDesc(&ScDesc);

	UINT Width, Height;
	Width = ScDesc.BufferDesc.Width;
	Height = ScDesc.BufferDesc.Height;

	//Z�o�b�t�@�p�̃e�N�X�`���쐬
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

	////�[�x�}�b�v�e�N�X�`���������_�[�^�[�Q�b�g�ɂ���ۂ̃f�v�X�X�e���V���r���[�p�̃e�N�X�`���[���쐬
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
//�p�[�e�B�N���V�X�e���I���ʒm�󂯎��
void ParticleSystemParent::OnNotify(Subject* subject_) {
	ParticleSystem* PSystem = reinterpret_cast<ParticleSystem*>(subject_);
	std::vector<int> nextNumbers = PSystem->getNextSystemNumbers();

	//���ɋN������p�[�e�B�N���V�X�e�����p�[�e�B�N���V�X�e���Q����Ăяo���ċN��
	for (int num = 0; num < nextNumbers.size(); num++) {
		m_ParticleSystemDictionary[nextNumbers[num]]->SetActive(true).SetEmitte(false).Start();
	}
}