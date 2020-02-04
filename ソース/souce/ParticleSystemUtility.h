#pragma once
#include	<d3d11.h>
#include	<d3dcompiler.h>
#include	<directxmath.h>
#include	<locale.h>
#include	<d3d11.h>
#include	<DirectXMath.h>
#include	<memory>
#include	<string>
#include    <vector>
//---------------------------------------------
//�p�[�e�B�N���V�X�e���̕⏕���\�b�h
//---------------------------------------------

namespace ParticleSystemUtility {
	std::string ExtractExtension(std::string fullpath);
	bool readShader(const char* csoName, std::vector<unsigned char>& byteArray);
	HRESULT CompileShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, void** ShaderObject, size_t& ShaderObjectSize, ID3DBlob** ppBlobOut);
	void CreateRWViewBuffer(ID3D11Device* device, ID3D11Buffer** VertexBuffer, ID3D11UnorderedAccessView** VertexView);
	//�R���X�^���g�o�b�t�@�쐬
	bool CreateConstantBuffer(ID3D11Device* device, unsigned int bytesize, ID3D11Buffer** pConstantBuffer);
	bool CreateConstantBufferWrite(ID3D11Device* device, unsigned int bytesize, ID3D11Buffer** pConstantBuffer);
	//�C���f�b�N�X�o�b�t�@�쐬
	bool CreateIndexBuffer(ID3D11Device* device, unsigned int indexnum, void* indexdata, ID3D11Buffer** pIndexBuffer);

	//���_�o�b�t�@�쐬
	bool CreateVertexBuffer(ID3D11Device* device, unsigned int stride, unsigned int vertexnum, void* vertexdata, ID3D11Buffer** pVertexBuffer);
	bool CreateVertexBufferWrite(ID3D11Device* device,unsigned int stride,unsigned int vertexnum,void* vertexdata,ID3D11Buffer** pVertexBuffer);
	bool CreateVertexBufferUAV(ID3D11Device* device,unsigned int stride,unsigned int vertexnum,void* vertexdata,ID3D11Buffer** pVertexBuffer);

	//�X�g���N�`���[�h�o�b�t�@�쐬
	bool CreateStructuredBuffer(ID3D11Device* device, unsigned int stride, unsigned int num, void* data, ID3D11Buffer** pStructuredBuffer);

	//�ʃo�b�t�@����R�s�[���č쐬
	ID3D11Buffer* CreateAndCopyToBuffer(ID3D11Device* device, ID3D11DeviceContext* devicecontext, ID3D11Buffer* pBuffer);

	//�V�F�[�_�[���\�[�X�r���[�쐬
	bool CreateShaderResourceView(ID3D11Device* device, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRV);
	bool CreatetSRVfromTGAFile(const char* filename, ID3D11Device* device, ID3D11ShaderResourceView** srv);
	bool CreatetSRVfromWICFile(const char* filename, ID3D11Device* device, ID3D11DeviceContext*	device11Context, ID3D11ShaderResourceView** srv);
	bool CreatetSRVfromDDS(const char* filename, ID3D11Device* device, ID3D11DeviceContext*	device11Context, ID3D11ShaderResourceView** srv);
	bool CreatetSRVfromFile(const char* filename, ID3D11Device* device, ID3D11DeviceContext*	device11Context, ID3D11ShaderResourceView** srv);

	//�A���I�[�_�[�h�r���[�쐬
	bool CreateUnOrderAccessView(ID3D11Device* device, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAV);

	//�V�F�[�_�[�쐬
	bool CreateVertexShader(ID3D11Device* device, const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, D3D11_INPUT_ELEMENT_DESC* layout, unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout**  ppVertexLayout);
	bool CreatePixelShader(ID3D11Device* device, const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11PixelShader** ppPixelShader);
	bool CreateComputeShader(ID3D11Device* device, const char* szFileName, LPCSTR szEmtryPoint, LPCSTR szShaderModel, ID3D11ComputeShader** ppComputeShader);

	//�V�F�[�_�[�R���p�C��
	HRESULT CompileShaderFromFile(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	//�R���s���[�g�V�F�[�_�[���s
	void RunComputeShader(ID3D11DeviceContext* pContext, ID3D11ComputeShader* pComputeShader, UINT numVIews, ID3D11ShaderResourceView** pSRVs, ID3D11UnorderedAccessView* pUAV, UINT x, UINT y, UINT z);
	void RunComputeShader(ID3D11DeviceContext* pContext, ID3D11ComputeShader* pComputeShader, UINT numVIews, ID3D11ShaderResourceView** pSRVs, ID3D11Buffer* pCBCS, void* pCSData, DWORD numDaraBytes, ID3D11UnorderedAccessView* pUAV, UINT x, UINT y, UINT z);
}