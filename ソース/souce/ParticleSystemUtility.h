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
//パーティクルシステムの補助メソッド
//---------------------------------------------

namespace ParticleSystemUtility {
	std::string ExtractExtension(std::string fullpath);
	bool readShader(const char* csoName, std::vector<unsigned char>& byteArray);
	HRESULT CompileShader(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, void** ShaderObject, size_t& ShaderObjectSize, ID3DBlob** ppBlobOut);
	void CreateRWViewBuffer(ID3D11Device* device, ID3D11Buffer** VertexBuffer, ID3D11UnorderedAccessView** VertexView);
	//コンスタントバッファ作成
	bool CreateConstantBuffer(ID3D11Device* device, unsigned int bytesize, ID3D11Buffer** pConstantBuffer);
	bool CreateConstantBufferWrite(ID3D11Device* device, unsigned int bytesize, ID3D11Buffer** pConstantBuffer);
	//インデックスバッファ作成
	bool CreateIndexBuffer(ID3D11Device* device, unsigned int indexnum, void* indexdata, ID3D11Buffer** pIndexBuffer);

	//頂点バッファ作成
	bool CreateVertexBuffer(ID3D11Device* device, unsigned int stride, unsigned int vertexnum, void* vertexdata, ID3D11Buffer** pVertexBuffer);
	bool CreateVertexBufferWrite(ID3D11Device* device,unsigned int stride,unsigned int vertexnum,void* vertexdata,ID3D11Buffer** pVertexBuffer);
	bool CreateVertexBufferUAV(ID3D11Device* device,unsigned int stride,unsigned int vertexnum,void* vertexdata,ID3D11Buffer** pVertexBuffer);

	//ストラクチャードバッファ作成
	bool CreateStructuredBuffer(ID3D11Device* device, unsigned int stride, unsigned int num, void* data, ID3D11Buffer** pStructuredBuffer);

	//別バッファからコピーして作成
	ID3D11Buffer* CreateAndCopyToBuffer(ID3D11Device* device, ID3D11DeviceContext* devicecontext, ID3D11Buffer* pBuffer);

	//シェーダーリソースビュー作成
	bool CreateShaderResourceView(ID3D11Device* device, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRV);
	bool CreatetSRVfromTGAFile(const char* filename, ID3D11Device* device, ID3D11ShaderResourceView** srv);
	bool CreatetSRVfromWICFile(const char* filename, ID3D11Device* device, ID3D11DeviceContext*	device11Context, ID3D11ShaderResourceView** srv);
	bool CreatetSRVfromDDS(const char* filename, ID3D11Device* device, ID3D11DeviceContext*	device11Context, ID3D11ShaderResourceView** srv);
	bool CreatetSRVfromFile(const char* filename, ID3D11Device* device, ID3D11DeviceContext*	device11Context, ID3D11ShaderResourceView** srv);

	//アンオーダードビュー作成
	bool CreateUnOrderAccessView(ID3D11Device* device, ID3D11Buffer* pBuffer, ID3D11UnorderedAccessView** ppUAV);

	//シェーダー作成
	bool CreateVertexShader(ID3D11Device* device, const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, D3D11_INPUT_ELEMENT_DESC* layout, unsigned int numElements, ID3D11VertexShader** ppVertexShader, ID3D11InputLayout**  ppVertexLayout);
	bool CreatePixelShader(ID3D11Device* device, const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3D11PixelShader** ppPixelShader);
	bool CreateComputeShader(ID3D11Device* device, const char* szFileName, LPCSTR szEmtryPoint, LPCSTR szShaderModel, ID3D11ComputeShader** ppComputeShader);

	//シェーダーコンパイル
	HRESULT CompileShaderFromFile(const char* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	//コンピュートシェーダー実行
	void RunComputeShader(ID3D11DeviceContext* pContext, ID3D11ComputeShader* pComputeShader, UINT numVIews, ID3D11ShaderResourceView** pSRVs, ID3D11UnorderedAccessView* pUAV, UINT x, UINT y, UINT z);
	void RunComputeShader(ID3D11DeviceContext* pContext, ID3D11ComputeShader* pComputeShader, UINT numVIews, ID3D11ShaderResourceView** pSRVs, ID3D11Buffer* pCBCS, void* pCSData, DWORD numDaraBytes, ID3D11UnorderedAccessView* pUAV, UINT x, UINT y, UINT z);
}