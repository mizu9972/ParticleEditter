#pragma once

#include <list>
#include <unordered_map>
#include "Shader.h"
#include "CParticle.h"
#include "Observer.h"

//�p�[�e�B�N���V�X�e���̐e�I�u�W�F�N�g
//�t�@�C�����o�͂���̂͂��̃N���X����
//�����쐬������p�[�e�B�N���V�X�e�����܂Ƃ߂ĊǗ�����
class ParticleSystemParent :public Observer{
private: 
	std::unordered_map<int, ParticleSystem*> m_ParticleSystemDictionary;//�p�[�e�B�N���V�X�e���Q
	t_ParticleSystemState DefaultParticleState;//�����ݒ�

	//�R���s���[�g�V�F�[�_�[
	ID3D11ComputeShader* m_ComputeShader = nullptr;
	ID3D11ComputeShader* m_InitComputeShader = nullptr;

	int m_ParticleCounter = 0;
public:
	ParticleSystemParent() {
	}

	//��{����
	void Init();
	void UnInit();
	void Start();
	void Update();
	void Draw();

	//�t�@�C������
	void InputParticleSystem(const char* filename);

	//�p�[�e�B�N���V�X�e���ǉ�
	ParticleSystem* AddParticleSystem();
	ParticleSystem* AddParticleSystem(t_ParticleSystemState* setState);
	ParticleSystem* AddParticleSystem(t_ParticleSystemState* setState, std::vector<int>& setNumbers);
	//�p�[�e�B�N���V�X�e���폜
	void RemoveParticleSystem(ParticleSystem* pParticleSystem_);///�s�v
	void RemoveParticleSystem(int removeKey);
	void DeleteParticleSystem();//�S�폜

	//���z�֐�����
	virtual void OnNotify(Subject* subject_ = nullptr);

	//�A�N�Z�T
	void setParticleCounter(int set) { m_ParticleCounter = set; };
	std::unordered_map<int, ParticleSystem*> getParticleSystem() { return m_ParticleSystemDictionary; };
	int getParticleSystemNum() { return static_cast<int>(m_ParticleSystemDictionary.size()); };
	int getParticleSystemCount() { return m_ParticleCounter; };
};