#pragma once
#include	<d3d11.h>
#include	"DatFile.h"

class CModel {
private:
	DatFile					m_datfile;						// datファイル
	ID3D11VertexShader*     m_pVertexShader = nullptr;		// 頂点シェーダー入れ物
	ID3D11PixelShader*      m_pPixelShader = nullptr;		// ピクセルシェーダー入れ物
	ID3D11InputLayout*      m_pVertexLayout = nullptr;		// 頂点フォーマット定義

	ID3D11ShaderResourceView*	m_texSRV = nullptr;			// テクスチャＳＲＶ
public:
	bool Init(const char* filename,const char* vsfile,const char* psfile);
	void Uninit();
	void Update();
	void Draw();
	DatFileLoader* GetDatFileData() {
		return m_datfile.GetDatFileLoader();
	}
};
