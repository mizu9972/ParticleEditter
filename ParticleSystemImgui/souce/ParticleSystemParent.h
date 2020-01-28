#pragma once

#include <map>
#include "Shader.h"
#include "CParticle.h"
#include "Observer.h"

//パーティクルシステムの親オブジェクト
//ファイル入出力するのはこのクラスから
//複数作成し得るパーティクルシステムをまとめて管理する
class ParticleSystemParent :public Observer{
private:
	std::map<int, ParticleSystem*> m_ParticleSystemDictionary;//パーティクルシステム群

	//コンピュートシェーダー
	ID3D11ComputeShader* m_ComputeShader = nullptr;
	ID3D11ComputeShader* m_InitComputeShader = nullptr;

	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_Devicecontext = nullptr;

	ID3D11DepthStencilView* m_DepthstencilView = nullptr;
	ID3D11DepthStencilView* m_DepthstencilViewRTV = nullptr;
	ID3D11ShaderResourceView* m_DepthSRV = nullptr;
	IDXGISwapChain* m_SwapChain = nullptr;
	//レンダーターゲットビュー
	ID3D11RenderTargetView* m_RenderTargetView = nullptr;
	ID3D11RenderTargetView* m_BackRTV = nullptr;

	int m_ParticleCounter = 0;//パーティクルシステムそれぞれに持たせる固有の番号

	bool InitDepthBuffer(unsigned int Width, unsigned int Height, ID3D11DepthStencilView* DepthstencilView);

	//レンダーターゲットビューからシェーダーリソースビューを取得
	ID3D11ShaderResourceView* getSRVfromRTV(ID3D11RenderTargetView* ResourceView);
	HRESULT CreateRTV(ID3D11RenderTargetView** outRTV, DXGI_FORMAT format);
public:
	ParticleSystemParent() {
	}

	//基本処理
	void Init(ID3D11Device* device,ID3D11DeviceContext* devicecontext, ID3D11DepthStencilView* depthstencilView = nullptr, ID3D11RenderTargetView* RenderTargetView = nullptr, IDXGISwapChain* SwapChain = nullptr /*ID3D11DepthStencilView* depthstencilView = nullptr, ID3D11RenderTargetView* RenderTargetView = nullptr*/);
	void UnInit();
	void Start();
	void Update();
	void Draw();

	//ファイル入力
	void InputParticleSystem(const char* filename);

	//パーティクルシステム追加
	ParticleSystem* AddParticleSystem();
	ParticleSystem* AddParticleSystem(t_ParticleSystemState* setState);
	ParticleSystem* AddParticleSystem(t_ParticleSystemState* setState, std::vector<int>& setNumbers);

	//パーティクルシステム削除
	void RemoveParticleSystem(int removeKey);
	void DeleteParticleSystem();//全削除

	//Zバッファ操作
	void TurnOnZbuffer();
	void TurnOffZbuffer();

	//仮想関数実装
	virtual void OnNotify(Subject* subject_ = nullptr);

	//アクセサ
	void setParticleCounter(int set) { m_ParticleCounter = set; };
	std::map<int, ParticleSystem*> getParticleSystem() { return m_ParticleSystemDictionary; };
	int getParticleSystemNum() { return static_cast<int>(m_ParticleSystemDictionary.size()); };
	int getParticleSystemCount() { return m_ParticleCounter; };
};