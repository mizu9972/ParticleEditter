#pragma once
#include	<d3d11.h>

class CDirectXGraphics{
private:
	CDirectXGraphics() {}
	ID3D11Device*				m_lpDevice=nullptr;					// DIRECT3DDEVICE11デバイス
	ID3D11DeviceContext*		m_lpImmediateContext=nullptr;		// DIRECT3DDEVICE11デバイスコンテキスト
	IDXGISwapChain*				m_lpSwapChain=nullptr;				// スワップチェイン

	ID3D11RenderTargetView*		m_lpRenderTargetView = nullptr;		// レンダーターゲットビュー
	ID3D11Texture2D*			m_depthStencilBuffer = nullptr;		// Ｚバッファ、ステンシルバッファ
	ID3D11DepthStencilState*	m_depthStencilState = nullptr;		// Ｚバッファ、ステンシルステート
	ID3D11DepthStencilView*		m_depthStencilView = nullptr;		// Ｚバッファ、ステンシルビュー
	ID3D11RasterizerState*		m_rasterState = nullptr;			// ラスターステータス

	D3D_DRIVER_TYPE				m_DriverType;			// ドライバタイプ
	D3D_FEATURE_LEVEL			m_FeatureLevel;			// 機能レベル

	ID3D11BlendState*			m_alphaEnableBlendingState = nullptr;
	ID3D11BlendState*			m_alphaDisableBlendingState = nullptr;

	unsigned int				m_Width=0;				// バックバッファＸサイズ
	unsigned int				m_Height=0;				// バックバッファＹサイズ
public:

	CDirectXGraphics(const CDirectXGraphics&) = delete;
	CDirectXGraphics& operator=(const CDirectXGraphics&) = delete;
	CDirectXGraphics(CDirectXGraphics&&) = delete;
	CDirectXGraphics& operator=(CDirectXGraphics&&) = delete;

	static CDirectXGraphics* GetInstance() {
		static CDirectXGraphics instance;
		return &instance;
	}

	// 初期処理
	bool Init(HWND hWnd, unsigned int Width, unsigned int Height, bool fullscreen = false);
	
	// 終了処理
	void Exit();														

	// デバイスGET
	ID3D11Device*	GetDXDevice() const {							
		return m_lpDevice;
	}

	// イミディエイトデバイスGET
	ID3D11DeviceContext* GetImmediateContext() const {
		return m_lpImmediateContext;
	}

	// SWAPチェインGET
	IDXGISwapChain* GetSwapChain()const {
		return m_lpSwapChain;
	}

	// レンダリングターゲットGET
	ID3D11RenderTargetView* GetRenderTargetView() const{
		return m_lpRenderTargetView;
	}

	// depthstencil view
	ID3D11DepthStencilView* GetDepthStencilView() const{
		return m_depthStencilView;
	}

	// ビューポートの高さを取得する
	unsigned int GetViewPortHeight() const {
		return m_Height;
	}

	// ビューポートの幅を取得する
	unsigned int GetViewPortWidth() const {
		return m_Width;
	}

	// アルファブレンド有効化
	void TurnOnAlphaBlending();

	// アルファブレンド無効化
	void TurnOffAlphaBlending();
};
