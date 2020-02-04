#pragma once
#include	<d3d11.h>
#include	<DirectXMath.h>
#include	<math.h>
#include	<vector>
#include	"Shader.h"

const float PI = XM_PI;

class CSphere {
private:
	ID3D11Buffer*		m_pVertexBuffer;	// 頂点バッファ
	ID3D11Buffer*		m_pIndexBuffer;		// インデックスバッファ
	ID3D11VertexShader* m_pVertexShader;	// 頂点シェーダー
	ID3D11PixelShader*  m_pPixelShader;		// ピクセルシェーダー
	ID3D11InputLayout*  m_pVertexLayout;	// 頂点レイアウト
	unsigned int		m_divX;				// 水平方向の分割数
	unsigned int		m_divY;				// 垂直方向の分割数
	float				m_radius;			// 半径

	struct Vertex {
		DirectX::XMFLOAT3	Pos;
		DirectX::XMFLOAT3	Normal;
	};

	struct Face{
		unsigned int idx[3];
	};

	Vertex*					m_vertex;			// 頂点座標

	Face*					m_face;				// 面インデックスデータ
	unsigned int			m_facenum;			// 面数

	// インデックスデータを作成する
	void CreateIndex();
	// 頂点データを作成する
	void CreateVertex();

public:

	void Normalize(XMFLOAT3 vector, XMFLOAT3& Normal);

	void Draw(ID3D11DeviceContext* device);

	// 初期化
	bool Init(float r,						// 半径
		int division_horizontal,			// 水平方向の分割数
		int division_vertical,				// 垂直方向の分割数
		ID3D11Device* device);

	// 終了処理
	void Exit();
};