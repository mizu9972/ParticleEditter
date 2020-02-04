#include	"CSphere.h"
												// 法線ベクトルを計算
// ノーマライズ
void CSphere::Normalize(XMFLOAT3 vector, XMFLOAT3& Normal) {

	DirectX::XMVECTOR v;
	v = XMLoadFloat3(&vector);			// XMFLOAT3=>XMVECTOR
	v = XMVector3Normalize(v);			// 正規化
	XMStoreFloat3(&Normal, v);			// XMVECTOR=>XMFLOAT3
}

// インデックスデータを作成
void CSphere::CreateIndex() {

	// 面数セット
	m_facenum = m_divY * m_divX * 2;

	m_face = new Face[m_facenum];

	Face* pface = m_face;

	// インデックス生成
	for (unsigned int y = 0; y < m_divY; y++) {
		for (unsigned int x = 0; x < m_divX; x++) {
			int count = (m_divX + 1)*y + x;			// 左上座標のインデックス

			// 上半分
			pface->idx[0] = count;
			pface->idx[1] = count + 1;
			pface->idx[2] = count + 1 + (m_divX + 1);

			pface++;

			// 下半分
			pface->idx[0] = count;
			pface->idx[1] = count + (m_divX + 1) + 1;
			pface->idx[2] = count + (m_divX + 1);

			pface++;
		}
	}
}

// 頂点データを作成
void CSphere::CreateVertex() {

	float azimuth = 0.0f;			// 方位角
	float elevation = 0.0f;			// 仰角

	m_vertex = new Vertex[(m_divX + 1) * (m_divY + 1)];
	Vertex* pvtx = m_vertex;

	XMFLOAT3	Normal;
	// 方位角と仰角から球メッシュの頂点データを作成
	for (unsigned int y = 0; y <= m_divY; y++) {
		elevation = (PI * (float)y) / (float)m_divY;    // 仰角をセット
		float r = m_radius*sinf(elevation);					// 仰角に応じた半径を計算

		for (unsigned int x = 0; x <= m_divX; x++) {
			azimuth = (2 * PI * (float)x) / (float)m_divX;	// 方位角をセット

			// 頂点座標セット
			pvtx->Pos.x = r*cosf(azimuth);
			pvtx->Pos.y = m_radius*cosf(elevation);
			pvtx->Pos.z = r*sinf(azimuth);

			// 法線ベクトルセット
			Normalize(pvtx->Pos, Normal);		// 法線を計算
			pvtx->Normal = Normal;				// 法線をセット

			// 次へ
			pvtx++;
		}
	}
}

// 描画
void CSphere::Draw(ID3D11DeviceContext* device) {

	// 頂点バッファをセットする
	unsigned int stride = sizeof(Vertex);
	unsigned  offset = 0;
	device->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	device->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);			// インデックスバッファをセット
	device->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);		// トポロジーをセット（旧プリミティブタイプ）
	device->IASetInputLayout(m_pVertexLayout);					// 頂点レイアウトセット

	device->VSSetShader(m_pVertexShader, nullptr, 0);			// 頂点シェーダーをセット
	device->PSSetShader(m_pPixelShader, nullptr, 0);			// ピクセルシェーダーをセット

	device->DrawIndexed(m_facenum * 3,		// 描画するインデックス数（面数×３）
		0,									// 最初のインデックスバッファの位置
		0);									// 頂点バッファの最初から使う
}

bool CSphere::Init(float r,				// 半径
	int division_horizontal,			// 水平方向の分割数
	int division_vertical,				// 垂直方向の分割数
	ID3D11Device* device) {
		bool sts;
		// 分割数を保存
	m_divX = division_horizontal;
	m_divY = division_vertical;

	// 半径を保存
	m_radius = r;

	// 球のインデックスデータを作成
	CreateIndex();

	// 球の頂点データを作成
	CreateVertex();

	// 頂点バッファ作成
	sts = CreateVertexBufferWrite(
		device,
		sizeof(Vertex),						// １頂点当たりバイト数
		(m_divY + 1)*(m_divX + 1),			// 頂点数
		m_vertex,							// 頂点データ格納メモリ先頭アドレス
		&m_pVertexBuffer);					// 頂点バッファ
	if (!sts) {
		MessageBox(NULL, "CreateBuffer(vertex buffer) error", "Error", MB_OK);
		return false;
	}

	// インデックスバッファ作成
	sts = CreateIndexBuffer(
			device,
			(m_divY)*(m_divX) * 2 * 3,	// インデックス数
			m_face,					// インデックスデータ先頭アドレス
			&m_pIndexBuffer);		// インデックスバッファ
	if (!sts) {
		MessageBox(NULL, "CreateBuffer(index buffer) error", "Error", MB_OK);
		return false;
	}

	// 頂点データの定義
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	unsigned int numElements = ARRAYSIZE(layout);

	// 頂点シェーダーオブジェクトを生成、同時に頂点レイアウトも生成
	sts = CreateVertexShader(device,
		"shader/3dspherevs.fx",
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
		device,							// デバイスオブジェクト
		"shader/3dsphereps.fx",
		"main",
		"ps_4_0",
		&m_pPixelShader);
	if (!sts) {
		MessageBox(nullptr, "CreatePixelShader error", "error", MB_OK);
		return false;
	}

	return true;
}

void CSphere::Exit() {

	if (m_vertex) {
		delete[] m_vertex;
	}

	if (m_face) {
		delete[] m_face;
	}

	// ピクセルシェーダー解放
	if (m_pPixelShader) {
		m_pPixelShader->Release();
		m_pPixelShader = nullptr;
	}

	// 頂点シェーダー解放
	if (m_pVertexShader) {
		m_pVertexShader->Release();
		m_pVertexShader = nullptr;
	}

	if (m_pVertexLayout) {
		m_pVertexLayout->Release();
		m_pVertexLayout = nullptr;
	}

	if (m_pIndexBuffer) {
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}

	if (m_pVertexBuffer) {
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}
}