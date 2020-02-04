#include	<d3d11.h>
#include	<DirectXMath.h>
#include	"CDirectxGraphics.h"
#include	"Shader.h"
#include	"CCamera.h"
#include	"memory.h"
#include	"CLight.h"
#include	"DX11Settransform.h"

										// 定数バッファ定義
ID3D11Buffer*       g_pConstantBuffer1 = nullptr;	// コンスタントバッファ1

CLight				g_directionallight;

void DX11LightInit(DirectX::XMFLOAT4 lightpos) {
	g_directionallight.Init(CCamera::GetInstance()->GetEye(), lightpos);
}

void DX11LightUninit() {
	g_directionallight.Uninit();
}

void DX11LightUpdate(DirectX::XMFLOAT4 lpos) {
	g_directionallight.SetEyePos(CCamera::GetInstance()->GetEye());
	g_directionallight.SetLightPos(lpos);
	g_directionallight.Update();
}

ID3D11Device* GetDX11Device() {
	return CDirectXGraphics::GetInstance()->GetDXDevice();
}

ID3D11DeviceContext*	GetDX11DeviceContext() {
	return CDirectXGraphics::GetInstance()->GetImmediateContext();
}

bool DX11Init(HWND hwnd, int width, int height, bool fullscreen) {
	bool sts;

	sts = CDirectXGraphics::GetInstance()->Init(hwnd, width, height, fullscreen);
	if (!sts) {
		MessageBox(hwnd, "DX11 init error", "error", MB_OK);
		return false;
	}

	//CDirectXGraphics::GetInstance()->TurnOnAlphaBlending();

	sts = DX11SetTransform::GetInstance()->Init();
	if (!sts) {
		MessageBox(NULL, "SetTransform error", "Error", MB_OK);
		return false;
	}

	return true;
}

void DX11Uninit() {

	DX11SetTransform::GetInstance()->Uninit();

	// 解放処理
	CDirectXGraphics::GetInstance()->Exit();

}

void DX11BeforeRender(float ClearColor[]) {
	// ターゲットバッファクリア
	CDirectXGraphics::GetInstance()->GetImmediateContext()->ClearRenderTargetView(CDirectXGraphics::GetInstance()->GetRenderTargetView(), ClearColor);
	// Zバッファクリア
	CDirectXGraphics::GetInstance()->GetImmediateContext()->ClearDepthStencilView(CDirectXGraphics::GetInstance()->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DX11AfterRender() {
	// ｓｗａｐ
	CDirectXGraphics::GetInstance()->GetSwapChain()->Present(0, 0);
}

ID3D11RenderTargetView* DX11GetRenderTagetView() {
	return CDirectXGraphics::GetInstance()->GetRenderTargetView();
}

