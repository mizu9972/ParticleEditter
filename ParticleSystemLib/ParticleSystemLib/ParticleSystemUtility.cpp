#include "stdafx.h"
#include "ParticleSystemUtility.h"
#include	<d3dcompiler.h>
#include	<directxmath.h>
#include	<locale.h>
#include	<d3d11.h>
#include	<DirectXMath.h>
#include	<memory>
#include	"directXtex.h"
#include	"WICTextureLoader.h"
#include	"DDSTextureLoader.h"
//---------------------------------------------
//�p�[�e�B�N���V�X�e���̕⏕���\�b�h
//---------------------------------------------

bool ParticleSystemUtility::CreateConstantBuffer(ID3D11Device* device, unsigned int bytesize, ID3D11Buffer** pConstantBuffer) {

	// �R���X�^���g�o�b�t�@����
	D3D11_BUFFER_DESC bd;

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;								// �o�b�t�@�g�p���@
	bd.ByteWidth = bytesize;									// �o�b�t�@�̑傫
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;					// �R���X�^���g�o�b�t�@
	bd.CPUAccessFlags = 0;										// CPU�A�N�Z�X�s�v

	HRESULT hr = device->CreateBuffer(&bd, nullptr, pConstantBuffer);
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateBuffer(constant buffer) error", "Error", MB_OK);
		return false;
	}

	return true;
}
// �g���q���擾����
std::string ParticleSystemUtility::ExtractExtension(std::string fullpath) {
	size_t ext_i = fullpath.find_last_of(".");
	std::string extname = fullpath.substr(ext_i + 1, fullpath.size() - ext_i);
	return extname;
}
//--------------------------------------------------------------------------------------
// �R���p�C���ς݃V�F�[�_�[�t�@�C����ǂݍ���
//--------------------------------------------------------------------------------------
bool ParticleSystemUtility::readShader(const char* csoName, std::vector<unsigned char>& byteArray)
{
	FILE* fp;
	int ret = fopen_s(&fp, csoName, "rb");
	if (ret != 0) {
		return false;
	}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	byteArray.resize(size);
	fseek(fp, 0, SEEK_SET);

	fread(byteArray.data(), byteArray.size(), 1, fp);
	fclose(fp);

	return true;
}

//--------------------------------------------------------------------------------------
// �V�F�[�_�[���t�@�C���g���q�ɍ��킹�ăR���p�C��
//--------------------------------------------------------------------------------------
HRESULT ParticleSystemUtility::CompileShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, void** ShaderObject, size_t& ShaderObjectSize, ID3DBlob** ppBlobOut) {

	HRESULT hr;
	static std::vector<unsigned char> byteArray;
	*ppBlobOut = nullptr;

	std::string extname = ExtractExtension(szFileName);
	if (extname == "cso") {
		bool sts = readShader(szFileName, byteArray);
		if (!sts) {
			FILE* fp;
			fopen_s(&fp, "debug.txt", "a");
			fprintf(fp, "file open error \n");
			fclose(fp);
			return E_FAIL;
		}
		*ShaderObject = byteArray.data();
		ShaderObjectSize = byteArray.size();
	}
	else {
		hr = CompileShaderFromFile(szFileName, szEntryPoint, szShaderModel, ppBlobOut);
		if (FAILED(hr)) {
			if (*ppBlobOut)(*ppBlobOut)->Release();
			return E_FAIL;
		}
		*ShaderObject = (*ppBlobOut)->GetBufferPointer();
		ShaderObjectSize = (*ppBlobOut)->GetBufferSize();
	}

	return S_OK;
}
//--------------------------------------------------------------------------------------
// �V�F�[�_�[���R���p�C��
//--------------------------------------------------------------------------------------
HRESULT ParticleSystemUtility::CompileShaderFromFile(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	ID3DBlob* p1 = nullptr;

	HRESULT hr = S_OK;

	WCHAR	filename[512];
	size_t 	wLen = 0;
	int err = 0;

	// char -> wchar�ɕϊ�
	setlocale(LC_ALL, "japanese");
	err = mbstowcs_s(&wLen, filename, 512, szFileName, _TRUNCATE);

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	hr = D3DCompileFromFile(
		filename,							// filename LPCWST pFileName
		nullptr,							// D3D_SHADER_MACRO *pDefines
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	// ID3DInclude *pInclude
		szEntryPoint,						// LPCSTR pEntrypoint
		szShaderModel,						// LPCSTR pTarget
		dwShaderFlags,						// UINT Flags1
		0,									// UINT Flags2
		ppBlobOut,							// ID3DBlob** ppCode
		&pErrorBlob);						// ID3DBlob** ppErrorMsg 
	if (FAILED(hr))
	{
		if (pErrorBlob != nullptr) {
			MessageBox(NULL,
				(char*)pErrorBlob->GetBufferPointer(), "Error", MB_OK);
		}
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

//--------------------------------------------------------------------------------------
// ���_�V�F�[�_�[�I�u�W�F�N�g�𐶐�����
//--------------------------------------------------------------------------------------
bool ParticleSystemUtility::CreateVertexShader(ID3D11Device* device,
	const char* szFileName,
	LPCSTR szEntryPoint,
	LPCSTR szShaderModel,
	D3D11_INPUT_ELEMENT_DESC* layout,
	unsigned int numElements,
	ID3D11VertexShader** ppVertexShader,
	ID3D11InputLayout**  ppVertexLayout) {

	HRESULT hr;

	ID3DBlob* pBlob = nullptr;

	void* ShaderObject;
	size_t	ShaderObjectSize;

	// �t�@�C���̊g���q�ɍ��킹�ăR���p�C��
	hr = ParticleSystemUtility::CompileShader(szFileName, szEntryPoint, szShaderModel, &ShaderObject, ShaderObjectSize, &pBlob);
	if (FAILED(hr))
	{
		if (pBlob)pBlob->Release();
		return false;
	}

	// ���_�V�F�[�_�[�𐶐�
	hr = device->CreateVertexShader(ShaderObject, ShaderObjectSize, nullptr, ppVertexShader);
	if (FAILED(hr))
	{
		if (pBlob)pBlob->Release();
		return false;
	}

	// ���_�f�[�^��`����
	hr = device->CreateInputLayout(
		layout,
		numElements,
		ShaderObject,
		ShaderObjectSize,
		ppVertexLayout);
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateInputLayout error", "error", MB_OK);
		pBlob->Release();
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------
// �s�N�Z���V�F�[�_�[�I�u�W�F�N�g�𐶐�����
//--------------------------------------------------------------------------------------
bool ParticleSystemUtility::CreatePixelShader(ID3D11Device* device,
	const char* szFileName,
	LPCSTR szEntryPoint,
	LPCSTR szShaderModel,
	ID3D11PixelShader** ppPixelShader) {

	HRESULT hr;

	ID3DBlob* pBlob = nullptr;

	void* ShaderObject;
	size_t	ShaderObjectSize;

	// �t�@�C���̊g���q�ɍ��킹�ăR���p�C��
	hr = ParticleSystemUtility::CompileShader(szFileName, szEntryPoint, szShaderModel, &ShaderObject, ShaderObjectSize, &pBlob);
	if (FAILED(hr))
	{
		return false;
	}

	// �s�N�Z���V�F�[�_�[�𐶐�
	hr = device->CreatePixelShader(ShaderObject, ShaderObjectSize, nullptr, ppPixelShader);
	if (FAILED(hr))
	{
		if (pBlob)pBlob->Release();
		return false;
	}

	return true;
}
//--------------------------------------------------------------------------------------
// �R���s���[�g�V�F�[�_�[�I�u�W�F�N�g�𐶐�����
//--------------------------------------------------------------------------------------
bool ParticleSystemUtility::CreateComputeShader(ID3D11Device* device,
	const char* szFileName,
	LPCSTR szEntryPoint,
	LPCSTR szShaderModel,
	ID3D11ComputeShader** ppComputeShader) {

	ID3DBlob* pBlob = nullptr;

	void* ShaderObject;
	size_t	ShaderObjectSize;
	//�R���p�C��
	HRESULT hr = ParticleSystemUtility::CompileShader(szFileName, szEntryPoint, szShaderModel, &ShaderObject, ShaderObjectSize, &pBlob);
	if (FAILED(hr)) {
		if (pBlob)pBlob->Release();
		return false;
	}
	//�R���s���[�g�V�F�[�_�[���쐬
	hr = device->CreateComputeShader(ShaderObject, ShaderObjectSize, nullptr, ppComputeShader);
	if (FAILED(hr)) {
		if (pBlob)pBlob->Release();
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------------
//�R���s���[�g�V�F�[�_�[�����s
//--------------------------------------------------------------------------------------
void ParticleSystemUtility::RunComputeShader(ID3D11DeviceContext* pContext,
	ID3D11ComputeShader* pComputeShader,
	UINT numVIews,//1
	ID3D11ShaderResourceView** pSRVs,
	ID3D11UnorderedAccessView* pUAV,
	UINT x, UINT y, UINT z
) {

	//�R���s���[�g�V�F�[�_�[�E
	//�V�F�[�_�[���\�[�X�r���[�E
	//�A���I�[�_�[�h�A�N�Z�X�r���[��ݒ�
	pContext->CSSetShader(pComputeShader, nullptr, 0);
	pContext->CSSetShaderResources(0, numVIews, pSRVs);
	pContext->CSSetUnorderedAccessViews(0, 1, &pUAV, nullptr);

	pContext->Dispatch(x, y, z);//���s

	ID3D11UnorderedAccessView* pNullUAVs[1] = { nullptr };
	ID3D11ShaderResourceView* pNullSRVs[2] = { nullptr,nullptr };
	ID3D11Buffer* pNullCBs[1] = { nullptr };

	pContext->CSSetShader(nullptr, nullptr, 0);
	pContext->CSSetUnorderedAccessViews(0, 1, pNullUAVs, nullptr);
	pContext->CSSetShaderResources(0, 2, pNullSRVs);
	pContext->CSSetConstantBuffers(0, 1, pNullCBs);
}
void ParticleSystemUtility::RunComputeShader(ID3D11DeviceContext* pContext,
	ID3D11ComputeShader* pComputeShader,
	UINT numVIews,//1
	ID3D11ShaderResourceView** pSRVs,
	ID3D11Buffer* pCBCS,
	void* pCSData,
	DWORD numDaraBytes,
	ID3D11UnorderedAccessView* pUAV,
	UINT x, UINT y, UINT z
) {
	//�R���s���[�g�V�F�[�_�[�E
	//�V�F�[�_�[���\�[�X�r���[�E
	//�A���I�[�_�[�h�A�N�Z�X�r���[��ݒ�
	pContext->CSSetShader(pComputeShader, nullptr, 0);
	pContext->CSSetShaderResources(0, numVIews, pSRVs);
	pContext->CSSetUnorderedAccessViews(0, 1, &pUAV, nullptr);

	if (pCBCS) {
		D3D11_MAPPED_SUBRESOURCE res;

		pContext->Map(pCBCS, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		memcpy(res.pData, pCSData, numDaraBytes);
		pContext->Unmap(pCBCS, 0);

		ID3D11Buffer* ppCB[1] = { pCBCS };
		pContext->CSSetConstantBuffers(0, 1, ppCB);
	}

	pContext->Dispatch(x, y, z);//���s

	ID3D11UnorderedAccessView* pNullUAVs[1] = { nullptr };
	ID3D11ShaderResourceView* pNullSRVs[2] = { nullptr,nullptr };
	ID3D11Buffer* pNullCBs[1] = { nullptr };

	pContext->CSSetShader(nullptr, nullptr, 0);
	pContext->CSSetUnorderedAccessViews(0, 1, pNullUAVs, nullptr);
	pContext->CSSetShaderResources(0, 2, pNullSRVs);
	pContext->CSSetConstantBuffers(0, 1, pNullCBs);
}
//--------------------------------------------------------------------------------------
//RW�o�b�t�@�r���[�I�u�W�F�N�g���쐬
//--------------------------------------------------------------------------------------
void ParticleSystemUtility::CreateRWViewBuffer(ID3D11Device* device, ID3D11Buffer** VertexBuffer, ID3D11UnorderedAccessView** VertexView) {


	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 0;
	uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

	device->CreateUnorderedAccessView(*VertexBuffer, &uavDesc, VertexView);//�������̌^�ɒ���
}
/*----------------------------
�R���X�^���g�o�b�t�@���쐬
------------------------------*/
bool ParticleSystemUtility::CreateConstantBufferWrite(
	ID3D11Device* device,					// �f�o�C�X�I�u�W�F�N�g
	unsigned int bytesize,					// �R���X�^���g�o�b�t�@�T�C�Y
	ID3D11Buffer** pConstantBuffer			// �R���X�^���g�o�b�t�@
) {

	// �R���X�^���g�o�b�t�@����
	D3D11_BUFFER_DESC bd;

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;							// �o�b�t�@�g�p���@
	bd.ByteWidth = bytesize;									// �o�b�t�@�̑傫
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;					// �R���X�^���g�o�b�t�@
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;					// CPU�A�N�Z�X�\

	HRESULT hr = device->CreateBuffer(&bd, nullptr, pConstantBuffer);
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateBuffer(constant buffer) error", "Error", MB_OK);
		return false;
	}

	return true;
}
/*------------------------
�C���f�b�N�X�o�b�t�@���쐬
--------------------------*/
bool ParticleSystemUtility::CreateIndexBuffer(
	ID3D11Device* device,						// �f�o�C�X�I�u�W�F�N�g
	unsigned int indexnum,						// �C���f�b�N�X��
	void* indexdata,							// �C���f�b�N�X�f�[�^�i�[�������擪�A�h���X
	ID3D11Buffer** pIndexBuffer) {				// �C���f�b�N�X�o�b�t�@

	// �C���f�b�N�X�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	D3D11_SUBRESOURCE_DATA InitData;

	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;								// �o�b�t�@�g�p��
	bd.ByteWidth = sizeof(unsigned int) * indexnum;				// �o�b�t�@�̑傫
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;						// �C���f�b�N�X�o�b�t�@
	bd.CPUAccessFlags = 0;										// CPU�A�N�Z�X�s�v

	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indexdata;

	HRESULT hr = device->CreateBuffer(&bd, &InitData, pIndexBuffer);
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateBuffer(index buffer) error", "Error", MB_OK);
		return false;
	}

	return true;
}

/*------------------------
���_�o�b�t�@���쐬
--------------------------*/
bool ParticleSystemUtility::CreateVertexBuffer(
	ID3D11Device* device,
	unsigned int stride,				// �P���_������o�C�g��
	unsigned int vertexnum,				// ���_��
	void* vertexdata,					// ���_�f�[�^�i�[�������擪�A�h���X
	ID3D11Buffer** pVertexBuffer		// ���_�o�b�t�@
) {

	HRESULT hr;

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;							// �o�b�t�@�g�p���@
	bd.ByteWidth = stride * vertexnum;						// �o�b�t�@�̑傫��
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;				// ���_�o�b�t�@
	bd.CPUAccessFlags = 0;									// CPU�A�N�Z�X�s�v

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertexdata;							// �o�b�t�@�̏����l

	hr = device->CreateBuffer(&bd, &InitData, pVertexBuffer);		// �o�b�t�@����
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateBuffer(vertex buffer) error", "Error", MB_OK);
		return false;
	}

	return true;
}

/*------------------------
���_�o�b�t�@���쐬(�b�o�t�������݉\)
--------------------------*/
bool ParticleSystemUtility::CreateVertexBufferWrite(
	ID3D11Device* device,
	unsigned int stride,				// �P���_������o�C�g��
	unsigned int vertexnum,				// ���_��
	void* vertexdata,					// ���_�f�[�^�i�[�������擪�A�h���X
	ID3D11Buffer** pVertexBuffer		// ���_�o�b�t�@
) {

	HRESULT hr;

	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;							// �o�b�t�@�g�p���@
	bd.ByteWidth = stride * vertexnum;						// �o�b�t�@�̑傫��
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;				// ���_�o�b�t�@
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;				// CPU�A�N�Z�X�\

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertexdata;							// �o�b�t�@�̏����l

	hr = device->CreateBuffer(&bd, &InitData, pVertexBuffer);		// �o�b�t�@����
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateBuffer(vertex buffer) error", "Error", MB_OK);
		return false;
	}

	return true;
}

/*------------------------
���_�o�b�t�@(UAV)���쐬
--------------------------*/
bool ParticleSystemUtility::CreateVertexBufferUAV(
	ID3D11Device* device,
	unsigned int stride,				// �P���_������o�C�g��
	unsigned int vertexnum,				// ���_��
	void* vertexdata,					// ���_�f�[�^�i�[�������擪�A�h���X
	ID3D11Buffer** pVertexBuffer		// ���_�o�b�t�@
) {

	HRESULT hr;

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.ByteWidth = stride * vertexnum;						// �o�b�t�@�̑傫��
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags =
		D3D11_BIND_VERTEX_BUFFER |
		D3D11_BIND_SHADER_RESOURCE |
		D3D11_BIND_UNORDERED_ACCESS;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertexdata;							// �o�b�t�@�̏����l

	hr = device->CreateBuffer(&bd, &InitData, pVertexBuffer);		// �o�b�t�@����
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateBuffer(vertex buffer) error", "Error", MB_OK);
		return false;
	}

	return true;
}

/*------------------------
 Structured�o�b�t�@���쐬
--------------------------*/
bool ParticleSystemUtility::CreateStructuredBuffer(
	ID3D11Device* device,
	unsigned int stride,				// �X�g���C�h�o�C�g��
	unsigned int num,					// ��
	void* data,							// �f�[�^�i�[�������擪�A�h���X
	ID3D11Buffer** pStructuredBuffer	// RWStructuredBuffer
) {

	HRESULT hr;

	// Structured�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;		// UA�o�b�t�@
	bd.ByteWidth = stride * num;													// �o�b�t�@�̑傫��
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;							// RWStructuredBuffer
	bd.StructureByteStride = stride;												// �\�����o�b�t�@�T�C�Y
	bd.CPUAccessFlags = 0;															// CPU�A�N�Z�X�s�v
	bd.Usage = D3D11_USAGE_DEFAULT;

	if (data != nullptr) {
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));

		InitData.pSysMem = data;							// �o�b�t�@�̏����l

		hr = device->CreateBuffer(&bd, &InitData, pStructuredBuffer);		// �o�b�t�@����
	}
	else {
		hr = device->CreateBuffer(&bd, nullptr, pStructuredBuffer);		// �o�b�t�@����
	}
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateBuffer(StructuredBuffer) error", "Error", MB_OK);
		return false;
	}

	return true;
}

/*---------------------------------
STAGING�o�b�t�@���쐬���R�s�[����
----------------------------------*/
ID3D11Buffer* ParticleSystemUtility::CreateAndCopyToBuffer(
	ID3D11Device* device,
	ID3D11DeviceContext* devicecontext,
	ID3D11Buffer* pBuffer	// RWStructuredBuffer
) {

	HRESULT hr;
	ID3D11Buffer* CloneBuffer = nullptr;

	// Structured�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	pBuffer->GetDesc(&bd);

	bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bd.Usage = D3D11_USAGE_STAGING;
	bd.BindFlags = 0;
	bd.MiscFlags = 0;

	hr = device->CreateBuffer(&bd, nullptr, &CloneBuffer);
	if (SUCCEEDED(hr)) {
		devicecontext->CopyResource(CloneBuffer, pBuffer);
	}

	return CloneBuffer;
}
/*------------------------
ShaderResourceView���쐬
--------------------------*/
bool ParticleSystemUtility::CreateShaderResourceView(
	ID3D11Device* device,
	ID3D11Buffer* pBuffer,	// Buffer
	ID3D11ShaderResourceView** ppSRV) {

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	pBuffer->GetDesc(&bd);

	D3D11_SHADER_RESOURCE_VIEW_DESC  srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;

	if (bd.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS) {
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		srvDesc.BufferEx.NumElements = bd.ByteWidth / 4;
	}
	else if (bd.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) {
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.BufferEx.NumElements = bd.ByteWidth / bd.StructureByteStride;
	}
	else {
		return false;
	}

	HRESULT hr;
	hr = device->CreateShaderResourceView(pBuffer, &srvDesc, ppSRV);
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateShaderResourceView error", "Error", MB_OK);
		return false;
	}

	return true;
}

/*------------------------
UnOrderedAccessView���쐬
--------------------------*/
bool ParticleSystemUtility::CreateUnOrderAccessView(
	ID3D11Device* device,
	ID3D11Buffer* pBuffer,	// Buffer
	ID3D11UnorderedAccessView** ppUAV) {

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	pBuffer->GetDesc(&bd);

	D3D11_UNORDERED_ACCESS_VIEW_DESC  uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;

	if (bd.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS) {
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		uavDesc.Buffer.NumElements = bd.ByteWidth / 4;
	}
	else if (bd.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) {
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.NumElements = bd.ByteWidth / bd.StructureByteStride;
	}
	else {
		return false;
	}

	HRESULT hr;
	hr = device->CreateUnorderedAccessView(pBuffer, &uavDesc, ppUAV);
	if (FAILED(hr)) {
		MessageBox(nullptr, "CreateUnorderedAccessView error", "Error", MB_OK);
		return false;
	}

	return true;
}
/*------------------------------------------------------------
�@�s�f�`�t�@�C����ǂݍ��݃V�F�[�_�[���\�[�X�r���[�\���쐬����
 --------------------------------------------------------------*/
bool ParticleSystemUtility::CreatetSRVfromTGAFile(const char* filename,
	ID3D11Device* device,
	ID3D11ShaderResourceView** srv) {

	HRESULT		hr;

	wchar_t ws[512];
	size_t ret;

	setlocale(LC_CTYPE, "jpn");
	mbstowcs_s(&ret, ws, 512, filename, _TRUNCATE);

	DirectX::TexMetadata meta;
	DirectX::GetMetadataFromTGAFile(ws, meta);

	std::unique_ptr<DirectX::ScratchImage> image(new DirectX::ScratchImage);
	hr = LoadFromTGAFile(ws, &meta, *image);
	if (FAILED(hr)) {
		//		MessageBox(nullptr, "Texture Load error", "error", MB_OK);
		return false;
	}

	// �V�F�[�_�[���\�[�X�r���[�쐬
	hr = DirectX::CreateShaderResourceView(device, image->GetImages(), image->GetImageCount(), meta, srv);
	if (FAILED(hr)) {
		//		MessageBox(nullptr, "CreateShaderResourceView error", "error", MB_OK);
		return false;
	}

	return true;
}

/*------------------------------------------------------------
�@WIC�t�@�C����ǂݍ��݃V�F�[�_�[���\�[�X�r���[�\���쐬����
 --------------------------------------------------------------*/
bool ParticleSystemUtility::CreatetSRVfromWICFile(const char* filename,
	ID3D11Device* device,
	ID3D11DeviceContext*	device11Context,
	ID3D11ShaderResourceView** srv) {

	ID3D11Resource*			texr = nullptr;		// �e�N�X�`�����\�[�X

	wchar_t ws[512];
	size_t ret;

	setlocale(LC_CTYPE, "jpn");
	mbstowcs_s(&ret, ws, 512, filename, _TRUNCATE);

	// �e�N�X�`���ǂݍ���
	HRESULT hr = DirectX::CreateWICTextureFromFile(
		device,
		device11Context,
		ws,
		&texr, srv);
	if (FAILED(hr)) {
		//		MessageBox(NULL, "CreateWICTextureFromFile", "Error", MB_OK);
		return false;
	}

	return true;
}

/*------------------------------------------------------------
�@DDS�t�@�C����ǂݍ��݃V�F�[�_�[���\�[�X�r���[�\���쐬����
 --------------------------------------------------------------*/
bool ParticleSystemUtility::CreatetSRVfromDDS(const char* filename,
	ID3D11Device* device,
	ID3D11DeviceContext*	device11Context,
	ID3D11ShaderResourceView** srv) {

	ID3D11Resource*			texr = nullptr;		// �e�N�X�`�����\�[�X

	wchar_t ws[512];
	size_t ret;

	setlocale(LC_CTYPE, "jpn");
	mbstowcs_s(&ret, ws, 512, filename, _TRUNCATE);

	// �e�N�X�`���ǂݍ���
	HRESULT hr = DirectX::CreateDDSTextureFromFile(
		device,
		device11Context,
		ws,
		&texr, srv);
	if (FAILED(hr)) {
		//		MessageBox(NULL, "CreateWICTextureFromFile", "Error", MB_OK);
		return false;
	}

	return true;
}

/*------------------------------------------------------------
�@�t�@�C����ǂݍ��݃V�F�[�_�[���\�[�X�r���[���쐬����
 --------------------------------------------------------------*/
bool ParticleSystemUtility::CreatetSRVfromFile(
	const char* filename,
	ID3D11Device* device,
	ID3D11DeviceContext*	device11Context,
	ID3D11ShaderResourceView** srv) {

	std::string fname(filename);

	std::string ext;

	// �g���q���擾����
	ext = ParticleSystemUtility::ExtractExtension(fname);

	// DDS�t�@�C��
	if (ext == "dds") {
		bool sts = CreatetSRVfromDDS(fname.c_str(),
			device,
			device11Context,
			srv);
		if (!sts) {
			MessageBox(nullptr, fname.c_str(), "texload error", MB_OK);
			return false;
		}
	}
	// TGA�t�@�C��
	else if (ext == "tga") {
		bool sts = CreatetSRVfromTGAFile(fname.c_str(),
			device,
			srv);
		if (!sts) {
			MessageBox(nullptr, fname.c_str(), "texload error", MB_OK);
			return false;
		}
	}
	// ���̑��t�@�C��
	else {
		bool sts = CreatetSRVfromWICFile(fname.c_str(),
			device,
			device11Context,
			srv);
		if (!sts) {
			MessageBox(nullptr, fname.c_str(), "texload error", MB_OK);
			return false;
		}
	}
	return true;
}