//*****************************************************************************
//!	@file	CDirect3DXFile.h
//!	@brief	
//!	@note	datファイルクラス
//!	@author
//*****************************************************************************
#pragma once

//-----------------------------------------------------------------------------
//	Include header files.
//-----------------------------------------------------------------------------
#include	<d3d11.h>
#include	<DirectXMath.h>
#include	"DatFileLoader.h"

using namespace DirectX;

namespace X{
	// 定数バッファ定義（マテリアル単位）
	struct ConstantBufferMaterial{
		XMFLOAT4	AmbientMaterial;		// 環境光の反射率
		XMFLOAT4	DiffuseMaterial;		// ディフューズ光の反射率
		XMFLOAT4	SpecularMaterial;		// スペキュラ光の反射率
	};
}

//=============================================================================
//!	@class	CDirect3DXFile
//!	@brief	Xファイルクラス
//=============================================================================
class DatFile{
private:
	DWORD						m_nummaterial;			// マテリアル数
	ID3D11Buffer*				m_pVertexBuffer;		// 頂点バッファ
	ID3D11Buffer*				m_pIndexBuffer;			// インデックスバッファ
	DatFileLoader*				m_datfile;				// datファイルデータ
	ID3D11ShaderResourceView**  m_srv;					// シェーダーリソースビュー
	ID3D11Buffer*				m_cb3;					// コンスタントバッファ(マテリアル用)

public:
	DatFile(){
		m_pVertexBuffer = nullptr;
		m_pIndexBuffer = nullptr;
	}

	virtual ~DatFile()
	{
		UnLoad();
	}
	bool Load(const char *xfilename, ID3D11Device* device11, ID3D11DeviceContext*	device11Context);
	void UnLoad();
	void Draw(ID3D11DeviceContext* device);
	DatFileLoader* GetDatFileLoader() {
		return m_datfile;
	}
};

//******************************************************************************
//	End of file.
//******************************************************************************