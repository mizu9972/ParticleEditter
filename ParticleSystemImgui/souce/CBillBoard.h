#pragma once
#include	<directxmath.h>
#include	<d3d11.h>
#include <DirectXMath.h>
#include "ParticleSystemUtility.h"

using namespace DirectX;
/*----------------------------------------------------------------------

	ビルボード描画クラス	

-----------------------------------------------------------------------*/

class CBillBoard {
	XMFLOAT4X4					m_mat;			// ビルボード用の行列
	float						m_x;			// ビルボードの位置座標
	float						m_y;
	float						m_z;
	float						m_XSize;		// ビルボードのＸサイズ
	float						m_YSize;		// ビルボードのＹサイズ
	XMFLOAT4					m_Color;		// 頂点カラー値
	ID3D11Device*				m_dev                 = nullptr;			// デバイス
	ID3D11DeviceContext*		m_devcontext          = nullptr;		// デバイスコンテキスト
	ID3D11ShaderResourceView* 	m_srv                 = nullptr;			// Shader Resourceviewテクスチャ
	ID3D11Buffer*				m_vbuffer             = nullptr;		// 頂点バッファ
	ID3D11BlendState*			m_pBlendStateSrcAlpha = nullptr;
	ID3D11BlendState*			m_pBlendStateOne      = nullptr;
	ID3D11BlendState*			m_pBlendStateDefault  = nullptr;
	ID3D11BlendState*			m_pBlendStateInv      = nullptr;
	ID3D11VertexShader*			m_pVertexShader       = nullptr;	// 頂点シェーダー入れ物
	ID3D11PixelShader*			m_pPixelShader        = nullptr;	// ピクセルシェーダー入れ物
	ID3D11InputLayout*			m_pVertexLayout       = nullptr;	// 頂点フォーマット定義

	ID3D11Buffer*				m_cb5 = nullptr;//コンスタントバッファ
	// 頂点フォーマット
	struct MyVertex {
		float				x, y, z;
		DirectX::XMFLOAT4	color;
		float				tu, tv;
	};

	MyVertex				m_Vertex[4];			// ビルボードの頂点座標
private:
	// ビルボードの頂点座標を計算
	void CalcVertex();
	// ビルボード用の行列を生成
	void CalcBillBoardMatrix(const DirectX::XMFLOAT4X4& cameramat);
	// ソースアルファを設定する
	void SetBlendStateSrcAlpha();
	// ブレンドステートを生成する
	void CreateBlendStateSrcAlpha();
	// 加算合成を設定する
	void SetBlendStateOne();
	// ブレンドステート（加算合成）を生成する
	void CreateBlendStateOne();
	// デフォルトのブレンドステートを設定する
	void SetBlendStateDefault();
	// デフォルトのブレンドステートを生成する
	void CreateBlendStateDefault();
	// 線形合成のブレンドステートを設定する
	void SetBlendStateInv();
	// 線形合成のブレンドステートを生成する
	void CreateBlendStateInv();
public:
	CBillBoard() :m_x(0), m_y(0), m_z(0), m_srv(nullptr), m_dev(nullptr), m_devcontext(nullptr) {
	};

	bool Init(ID3D11Device* device, ID3D11DeviceContext* devicecontext, float x, float y, float z, float xsize, float ysize, DirectX::XMFLOAT4 color, const char *psFilename, const char *vsFilename);
	bool InitInstancing(ID3D11Device* device, ID3D11DeviceContext* devicecontext, float x, float y, float z, float xsize, float ysize, DirectX::XMFLOAT4 color, const char *psFilename, const char *vsFilename);
	
	// デストラクタ
	virtual ~CBillBoard(){
		//#TODO 解放処理 ComオブジェクトをComポインタ化 もしくは SAFE_RELEASE
		if (m_vbuffer != nullptr) {
			m_vbuffer->Release();
			m_vbuffer = nullptr;
		}
		if (m_cb5 != nullptr) {
			m_cb5->Release();
			m_cb5 = nullptr;
		}
		if(m_srv!=nullptr){
			m_srv->Release();
			m_srv = nullptr;
		}

		if (m_pBlendStateOne != nullptr) {
			m_pBlendStateOne->Release();
			m_pBlendStateOne = nullptr;
		}

		if (m_pBlendStateSrcAlpha != nullptr) {
			m_pBlendStateSrcAlpha->Release();
			m_pBlendStateSrcAlpha = nullptr;
		}

		if (m_pBlendStateDefault != nullptr) {
			m_pBlendStateDefault->Release();
			m_pBlendStateDefault = nullptr;
		}

		if (m_pBlendStateInv != nullptr) {
			m_pBlendStateInv->Release();
			m_pBlendStateInv = nullptr;
		}

		if (m_pPixelShader != nullptr) {
			m_pPixelShader->Release();
			m_pPixelShader = nullptr;
		}

		if (m_pVertexShader != nullptr) {
			m_pVertexShader->Release();
			m_pVertexShader = nullptr;
		}

		if (m_pVertexLayout != nullptr) {
			m_pVertexLayout->Release();
			m_pVertexLayout = nullptr;
		}
	}

	// ＵＶ座標をセットする
	void SetUV(float u[],float v[]){
		m_Vertex[0].tu = u[0];
		m_Vertex[0].tv = v[0];

		m_Vertex[1].tu = u[1];
		m_Vertex[1].tv = v[1];

		m_Vertex[2].tu = u[2];
		m_Vertex[2].tv = v[2];

		m_Vertex[3].tu = u[3];
		m_Vertex[3].tv = v[3];

		CalcVertex();						// ビルボード用の頂点データ作成	
	}

	void SetDrawUtility();
	// 描画
	void Draw();
	// 位置を指定
	void SetPosition(float x,float y,float z);

	//描画するだけ
	void DrawOnly(const DirectX::XMFLOAT4X4 &cameramat, float angle);
	// ビルボードを描画
	void DrawBillBoard(const DirectX::XMFLOAT4X4& cameramat);

	// ビルボード描画加算合成
	void DrawBillBoardAdd(const DirectX::XMFLOAT4X4& cameramat);

	// ビルボードをZ軸を中心にして回転させて描画
	void DrawRotateBillBoard(const DirectX::XMFLOAT4X4 &cameramat, float radian);
	void DrawRotateBillBoardAlpha(const DirectX::XMFLOAT4X4 &cameramat, float angle);
	// サイズをセット
	void SetSize(float x, float y);

	// カラーをセット
	void SetColor(DirectX::XMFLOAT4 col);

	//インスタンス数をセット
	void SetInstanceNum(int Num);

	//	テクスチャ読み込み
	bool LoadTexTure(const char* filename);

};