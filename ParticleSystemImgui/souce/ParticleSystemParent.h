#pragma once

#include <list>
#include <unordered_map>
#include "Shader.h"
#include "CParticle.h"
#include "Observer.h"

//パーティクルシステムの親オブジェクト
//ファイル入出力するのはこのクラスから
//複数作成し得るパーティクルシステムをまとめて管理する
class ParticleSystemParent :public Observer{
private: 
	std::unordered_map<int, ParticleSystem*> m_ParticleSystemDictionary;//パーティクルシステム群
	t_ParticleSystemState DefaultParticleState;//初期設定

	//コンピュートシェーダー
	ID3D11ComputeShader* m_ComputeShader = nullptr;
	ID3D11ComputeShader* m_InitComputeShader = nullptr;

	int m_ParticleCounter = 0;
public:
	ParticleSystemParent() {
	}

	//基本処理
	void Init();
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
	void RemoveParticleSystem(ParticleSystem* pParticleSystem_);///不要
	void RemoveParticleSystem(int removeKey);
	void DeleteParticleSystem();//全削除

	//仮想関数実装
	virtual void OnNotify(Subject* subject_ = nullptr);

	//アクセサ
	void setParticleCounter(int set) { m_ParticleCounter = set; };
	std::unordered_map<int, ParticleSystem*> getParticleSystem() { return m_ParticleSystemDictionary; };
	int getParticleSystemNum() { return static_cast<int>(m_ParticleSystemDictionary.size()); };
	int getParticleSystemCount() { return m_ParticleCounter; };
};