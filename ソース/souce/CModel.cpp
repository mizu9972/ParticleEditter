#include	<windows.h>
#include	"CModel.h"
#include	"Shader.h"

#include	"DX11util.h"
#include	"CCamera.h"
#include	"DX11Settransform.h"

bool CModel::Init(const char* filename, const char* vsfile, const char* psfile) {

	bool sts;

	// 飛行機のモデルデータを読み込み
	sts = m_datfile.Load(filename, GetDX11Device(), GetDX11DeviceContext());
	if (!sts) {
		char str[128];
		sprintf_s(str, 128,"%s load ERROR!!", filename);
		MessageBox(nullptr, str, "error", MB_OK);
		return false;
	}

	// 飛行機用の頂点データの定義
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	unsigned int numElements = ARRAYSIZE(layout);

	// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	sts = CreateVertexShader(GetDX11Device(),
		vsfile,
		"main",
		"vs_4_0",
		layout,
		numElements,
		&m_pVertexShader,
		&m_pVertexLayout);
	if (!sts) {
		MessageBox(nullptr, "CreateVertexShader error", "error", MB_OK);
		return false;
	}

	// ピクセルシェーダーを生成
	sts = CreatePixelShader(			// ピクセルシェーダーオブジェクトを生成
		GetDX11Device(),		// デバイスオブジェクト
		psfile,
		"main",
		"ps_4_0",
		&m_pPixelShader);
	if (!sts) {
		MessageBox(nullptr, "CreatePixelShader error", "error", MB_OK);
		return false;
	}

	return true;
}

void CModel::Uninit() {
	// 頂点シェーダー解放
	if (m_pVertexShader) {
		m_pVertexShader->Release();
		m_pVertexShader = nullptr;
	}

	// ピクセルシェーダー解放
	if (m_pPixelShader) {
		m_pPixelShader->Release();
		m_pPixelShader = nullptr;
	}

	// 頂点レイアウト解放
	if (m_pVertexLayout) {
		m_pVertexLayout->Release();
		m_pVertexLayout = nullptr;
	}
}

void CModel::Update() {
}

void CModel::Draw() {
	// 頂点フォーマットをセット
	GetDX11DeviceContext()->IASetInputLayout(m_pVertexLayout);

	// 頂点シェーダーをセット
	GetDX11DeviceContext()->VSSetShader(m_pVertexShader, nullptr, 0);

	// ピクセルシェーダーをセット
	GetDX11DeviceContext()->PSSetShader(m_pPixelShader, nullptr, 0);

	// モデル描画
	m_datfile.Draw(GetDX11DeviceContext());
}