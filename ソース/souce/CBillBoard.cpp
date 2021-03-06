#include	"CBillBoard.h"
#include	"ParticleMathUtil.h"
#include	"ParticleSystemTransform.h"

// ビルボードの頂点座標を計算
void CBillBoard::CalcVertex(){
		m_Vertex[0].x = -m_XSize/2;
		m_Vertex[0].y = -m_YSize/2;
		m_Vertex[0].z =0.0f;
		m_Vertex[0].color = m_Color;

		m_Vertex[1].x = -m_XSize/2;
		m_Vertex[1].y =  m_YSize/2;
		m_Vertex[1].z =0.0f;
		m_Vertex[1].color = m_Color;

		m_Vertex[2].x = m_XSize / 2;
		m_Vertex[2].y = -m_YSize / 2;
		m_Vertex[2].z = 0;
		m_Vertex[2].color = m_Color;

		m_Vertex[3].x = m_XSize/2;
		m_Vertex[3].y = m_YSize/2;
		m_Vertex[3].z =0.0f;
		m_Vertex[3].color = m_Color;

		if (m_vbuffer==nullptr) {
			// 頂点バッファ作成（後で変更可能な）
			bool sts = ParticleSystemUtility::CreateVertexBufferWrite(m_dev, sizeof(MyVertex), 4, m_Vertex, &m_vbuffer);
			if (!sts) {
				MessageBox(nullptr, "create vertex buffer erro(CBillBoard)", "error", MB_OK);
			}
		}
		else {
			// 作成済みなら 内容を書き換える 
			D3D11_MAPPED_SUBRESOURCE pData;

			HRESULT hr = m_devcontext->Map(m_vbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pData);
			if (SUCCEEDED(hr)) {
				memcpy_s(pData.pData, pData.RowPitch, (void*)(m_Vertex), sizeof(MyVertex) * 4);
				m_devcontext->Unmap(m_vbuffer, 0);
			}
		}

}

// サイズをセット
void CBillBoard::SetSize(float x, float y){
	m_XSize = x;
	m_YSize = y;
	CalcVertex();
}

// カラー値をセット
void CBillBoard::SetColor(XMFLOAT4  col){
	m_Color = col;
	CalcVertex();
}

// 位置をセット
void CBillBoard::SetPosition(float x,float y,float z){
	m_x=x;
	m_y=y;
	m_z=z;
}

// ビルボード用の行列を生成
void CBillBoard::CalcBillBoardMatrix(const DirectX::XMFLOAT4X4& cameramat){

	m_mat._11 =cameramat._11;
	m_mat._12 =cameramat._21;
	m_mat._13 =cameramat._31;

	m_mat._21 =cameramat._12;
	m_mat._22 =cameramat._22; 
	m_mat._23 =cameramat._32;

	m_mat._31 =cameramat._13;
	m_mat._32 =cameramat._23; 
	m_mat._33 =cameramat._33;

	m_mat._41 = m_x;
	m_mat._42 = m_y;
	m_mat._43 = m_z;

	m_mat._14=0;
	m_mat._24=0;
	m_mat._34=0;
	m_mat._44=1;
}

// 描画
void CBillBoard::Draw() {
	// Ｚバッファ無効化
	//TurnOffZbuffer();

	// シェーダーリソースビューをピクセルシェーダーへセット
	m_devcontext->PSSetShaderResources(0, 1, &m_srv);

	// ワールド変換行列
	ParticleSystemSetTransform::GetInstance()->SetTransform(ParticleSystemSetTransform::TYPE::WORLD, m_mat);

	// 頂点バッファをセット
	unsigned int stride = sizeof(MyVertex);
	unsigned  offset = 0;

	// 頂点バッファをセット
	m_devcontext->IASetVertexBuffers(0, 1, &m_vbuffer, &stride, &offset);
	// インデックスバッファをセット
//	m_devcontext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	// トポロジーをセット
	m_devcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 頂点フォーマットをセット
	m_devcontext->IASetInputLayout(m_pVertexLayout);
	
	// 4角形描画
	m_devcontext->VSSetShader(m_pVertexShader, nullptr, 0);		// 頂点シェーダーをセット
	m_devcontext->PSSetShader(m_pPixelShader, nullptr, 0);		// ピクセルシェーダーをセット
	m_devcontext->Draw(4,0);									// 4頂点描画(頂点バッファのみで描画)

	// Ｚバッファ有効化
	//TurnOnZbuffer();
}
// ビルボードを描画
void CBillBoard::DrawBillBoard(const XMFLOAT4X4& cameramat){

	// カメラ行列から　ビルボード用のマトリックスを作成
	CalcBillBoardMatrix(cameramat);

	//アルファブレンディングをセットする
	//SetBlendStateSrcAlpha();

	// 描画
	Draw();

	//アルファブレンディングをセットする
	//SetBlendStateDefault();
}

// ビルボードを描画
void CBillBoard::DrawBillBoardAdd(const XMFLOAT4X4& cameramat){

	// カメラ行列から　ビルボード用のマトリックスを作成
	CalcBillBoardMatrix(cameramat);

	// 描画
	//アルファブレンディングをセットする
	SetBlendStateOne();

	// 描画
	Draw();

	//アルファブレンディングをセットする
	SetBlendStateDefault();

	//アルファブレンディングで半透明合成をセットする
}

void CBillBoard::SetBlendStateDefault() {
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_devcontext->OMSetBlendState(m_pBlendStateDefault, blendFactor, 0xffffffff);
}

void CBillBoard::SetBlendStateSrcAlpha() {
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_devcontext->OMSetBlendState(m_pBlendStateSrcAlpha, blendFactor, 0xffffffff);
}

void CBillBoard::SetBlendStateOne() {
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_devcontext->OMSetBlendState(m_pBlendStateOne, blendFactor, 0xffffffff);
}

void CBillBoard::SetBlendStateInv() {
	float blendFactor[4] = { 0.0f,0.0f,0.0f,0.0f };
	m_devcontext->OMSetBlendState(m_pBlendStateInv, blendFactor, 0xffffffff);
}
void CBillBoard::CreateBlendStateSrcAlpha(){
	
	D3D11_BLEND_DESC BlendDesc;

	ZeroMemory(&BlendDesc, sizeof(BlendDesc));
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;		// 配列の0番のみ使用

	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_DEST_COLOR;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_dev->CreateBlendState(&BlendDesc, &m_pBlendStateSrcAlpha);
}

void CBillBoard::CreateBlendStateOne() {

	D3D11_BLEND_DESC BlendDesc;

	ZeroMemory(&BlendDesc, sizeof(BlendDesc));
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;		// 配列の0番のみ使用

	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_dev->CreateBlendState(&BlendDesc, &m_pBlendStateOne);
}

void CBillBoard::CreateBlendStateDefault() {

	D3D11_BLEND_DESC BlendDesc;

	ZeroMemory(&BlendDesc, sizeof(BlendDesc));

	ZeroMemory(&BlendDesc, sizeof(BlendDesc));
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;		// 配列の0番のみ使用

	BlendDesc.RenderTarget[0].BlendEnable = FALSE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_dev->CreateBlendState(&BlendDesc, &m_pBlendStateDefault);
}

void CBillBoard::CreateBlendStateInv() {
	D3D11_BLEND_DESC BlendDesc;

	ZeroMemory(&BlendDesc, sizeof(BlendDesc));
	BlendDesc.AlphaToCoverageEnable = FALSE;
	BlendDesc.IndependentBlendEnable = FALSE;

	BlendDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	m_dev->CreateBlendState(&BlendDesc, &m_pBlendStateInv);
}

// ビルボードを描画(Ｚ軸で回転)
void CBillBoard::DrawRotateBillBoard(const DirectX::XMFLOAT4X4 &cameramat, float angle){

	// カメラ行列から　ビルボード用のマトリックスを作成
	CalcBillBoardMatrix(cameramat);

	// Ｚ軸回転行列を作成
	DirectX::XMFLOAT4X4 matRotZ;
	DirectX::XMFLOAT3 axisZ = { cameramat._13, cameramat._23, cameramat._33 };

	// 指定軸回転の行列を作成する
	ParticleSystemMathUtil::DX11MtxRotationAxis(axisZ, angle, matRotZ);

	// 行列を合成し、位置をセット
	ParticleSystemMathUtil::DX11MtxMultiply(m_mat, m_mat, matRotZ);
	m_mat._41 = m_x;
	m_mat._42 = m_y;
	m_mat._43 = m_z;
	
	//アルファブレンディングをセットする
//	SetBlendStateOne();

	// 描画
	Draw();

	//アルファブレンディングをセットする
//	SetBlendStateDefault();

}

void CBillBoard::DrawOnly(const DirectX::XMFLOAT4X4 &cameramat,float angle) {

	// カメラ行列から　ビルボード用のマトリックスを作成
	CalcBillBoardMatrix(cameramat);

	// Ｚ軸回転行列を作成
	DirectX::XMFLOAT4X4 matRotZ;
	DirectX::XMFLOAT3 axisZ = { cameramat._13, cameramat._23, cameramat._33 };

	// 指定軸回転の行列を作成する
	ParticleSystemMathUtil::DX11MtxRotationAxis(axisZ, angle, matRotZ);
	// 行列を合成し、位置をセット
	ParticleSystemMathUtil::DX11MtxMultiply(m_mat, m_mat, matRotZ);
	m_mat._41 = m_x;
	m_mat._42 = m_y;
	m_mat._43 = m_z;

	//アルファブレンディングをセットする
	SetBlendStateInv();

	// ワールド変換行列
	ParticleSystemSetTransform::GetInstance()->SetTransform(ParticleSystemSetTransform::TYPE::WORLD, m_mat);

	// 描画
	//Draw();
	m_devcontext->Draw(4, 0);

	//アルファブレンディングをリセットする
	SetBlendStateDefault();

}

void CBillBoard::SetDrawUtility() {

	//// Ｚバッファ無効化
	//void TurnOffZbuffer();

	// シェーダーリソースビューをピクセルシェーダーへセット
	m_devcontext->PSSetShaderResources(0, 1, &m_srv);

	// 頂点バッファをセット
	unsigned int stride = sizeof(MyVertex);
	unsigned  offset = 0;

	// 頂点バッファをセット
	m_devcontext->IASetVertexBuffers(0, 1, &m_vbuffer, &stride, &offset);
	// インデックスバッファをセット
//	m_devcontext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
	// トポロジーをセット
	m_devcontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// 頂点フォーマットをセット
	m_devcontext->IASetInputLayout(m_pVertexLayout);

	// 4角形描画
	m_devcontext->VSSetShader(m_pVertexShader, nullptr, 0);		// 頂点シェーダーをセット
	m_devcontext->PSSetShader(m_pPixelShader, nullptr, 0);		// ピクセルシェーダーをセット
}

// ビルボードを描画(Ｚ軸で回転)
void CBillBoard::DrawRotateBillBoardAlpha(const DirectX::XMFLOAT4X4 &cameramat, float angle) {

	// カメラ行列から　ビルボード用のマトリックスを作成
	CalcBillBoardMatrix(cameramat);

	// Ｚ軸回転行列を作成
	DirectX::XMFLOAT4X4 matRotZ;
	DirectX::XMFLOAT3 axisZ = { cameramat._13, cameramat._23, cameramat._33 };

	// 指定軸回転の行列を作成する
	ParticleSystemMathUtil::DX11MtxRotationAxis(axisZ, angle, matRotZ);
	// 行列を合成し、位置をセット
	ParticleSystemMathUtil::DX11MtxMultiply(m_mat, m_mat, matRotZ);
	m_mat._41 = m_x;
	m_mat._42 = m_y;
	m_mat._43 = m_z;

	//アルファブレンディングをセットする
	SetBlendStateOne();

	// 描画
	Draw();

	//アルファブレンディングをセットする
	SetBlendStateDefault();

}
//	テクスチャ読み込み
bool CBillBoard::LoadTexTure(const char* filename){

		// すでに読み込まれているテクスチャがあれば解放する
		if(m_srv!=nullptr){
			m_srv->Release();
			m_srv=nullptr;
		}

		// まずはＷＩＣファイルと考えて読み込む
		bool sts = ParticleSystemUtility::CreatetSRVfromWICFile(filename,
			m_dev, m_devcontext, &m_srv);
		if (sts == false) {
			// TGAファイルを試してみる
			sts = ParticleSystemUtility::CreatetSRVfromTGAFile(filename,
				m_dev, &m_srv);
			if (sts == false) {
				MessageBox(nullptr, "textureload error(CBillBoard LoadTexture)", "error", MB_OK);
				return false;
			}
		}

		return true;
}