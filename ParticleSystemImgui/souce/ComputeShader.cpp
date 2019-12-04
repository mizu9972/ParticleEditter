//#include <d3dx11.h>
//#include <stdio.h>
//#include <tchar.h>
//
//#pragma comment( lib, "dxguid.lib" )
//#pragma comment( lib, "d3d11.lib" )
//#pragma comment( lib, "d3dx11.lib" )
//
////--------------------------------------------------------------------------------------
////コンパイル済みコンピュートシェーダーからオブジェクト作成
////--------------------------------------------------------------------------------------
//void CreateComputeShader(ID3D11Device **device, const char* filename, ID3D11ComputeShader** ppComputeShader) {
//	ID3D10Blob* pBlob = NULL;
//	HRESULT hr;
//	UINT flag1 = D3D10_SHADER_ENABLE_STRICTNESS;
//
//	hr = D3DX11CompileFromFile(filename, NULL, NULL, "main", "cs_5_0", flag1, 0, NULL, &pBlob, NULL, NULL);
//	if (FAILED(hr)) {
//		return;
//	}
//
//	hr = (*device)->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, ppComputeShader);
//	if (FAILED(hr)) {
//		return;
//	}
//
//	hr = S_OK;
//}