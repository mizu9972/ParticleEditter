#pragma once

#include <list>
#include <unordered_map>
#include "Shader.h"
#include "CParticle.h"
#include "Observer.h"

//パーティクルシステムの親オブジェクト
//ファイル出力するのはこのクラスの情報
class ParticleSystemParent :public Observer{
private: 
	std::unordered_map<int, ParticleSystem*> m_ParticleSystemDictionary;
	t_ParticleSystemState DefaultParticleState;

	ID3D11ComputeShader* m_ComputeShader = nullptr;
	ID3D11ComputeShader* m_InitComputeShader = nullptr;

	int m_ParticleCounter = 0;
public:
	ParticleSystemParent() {
		Init();
	}
	void Init();
	void UnInit();
	void Start();
	void Update();
	void Draw();

	ParticleSystem* AddParticleSystem();
	ParticleSystem* AddParticleSystem(t_ParticleSystemState* setState);
	void RemoveParticleSystem(ParticleSystem* pParticleSystem_);///不要
	void RemoveParticleSystem(int removeKey);
	void DeleteParticleSystem();

	//仮想関数実装
	virtual void OnNotify(Subject* subject_ = nullptr);

	//アクセサ
	void setParticleCounter(int set) { m_ParticleCounter = set; };
	std::unordered_map<int, ParticleSystem*> getParticleSystem() { return m_ParticleSystemDictionary; };
	int getParticleSystemCount() { return m_ParticleCounter; };
};