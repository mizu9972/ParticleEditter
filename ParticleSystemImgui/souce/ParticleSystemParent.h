#pragma once

#include <map>
#include "Shader.h"
#include "CParticle.h"
#include "Observer.h"

//�p�[�e�B�N���V�X�e���̐e�I�u�W�F�N�g
//�t�@�C�����o�͂���̂͂��̃N���X����
//�����쐬������p�[�e�B�N���V�X�e�����܂Ƃ߂ĊǗ�����
class ParticleSystemParent :public Observer{
private:
	std::map<int, ParticleSystem*> m_ParticleSystemDictionary;//�p�[�e�B�N���V�X�e���Q

	//�R���s���[�g�V�F�[�_�[
	ID3D11ComputeShader* m_ComputeShader = nullptr;
	ID3D11ComputeShader* m_InitComputeShader = nullptr;

	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_Devicecontext = nullptr;

	ID3D11DepthStencilView* m_DepthstencilView = nullptr;
	ID3D11DepthStencilView* m_DepthstencilViewRTV = nullptr;
	ID3D11ShaderResourceView* m_DepthSRV = nullptr;
	IDXGISwapChain* m_SwapChain = nullptr;
	//�����_�[�^�[�Q�b�g�r���[
	ID3D11RenderTargetView* m_RenderTargetView = nullptr;
	ID3D11RenderTargetView* m_BackRTV = nullptr;

	int m_ParticleCounter = 0;//�p�[�e�B�N���V�X�e�����ꂼ��Ɏ�������ŗL�̔ԍ�

	bool InitDepthBuffer(unsigned int Width, unsigned int Height, ID3D11DepthStencilView* DepthstencilView);

	//�����_�[�^�[�Q�b�g�r���[����V�F�[�_�[���\�[�X�r���[���擾
	ID3D11ShaderResourceView* getSRVfromRTV(ID3D11RenderTargetView* ResourceView);
	HRESULT CreateRTV(ID3D11RenderTargetView** outRTV, DXGI_FORMAT format);
public:
	ParticleSystemParent() {
	}

	//��{����
	void Init(ID3D11Device* device,ID3D11DeviceContext* devicecontext, ID3D11DepthStencilView* depthstencilView = nullptr, ID3D11RenderTargetView* RenderTargetView = nullptr, IDXGISwapChain* SwapChain = nullptr /*ID3D11DepthStencilView* depthstencilView = nullptr, ID3D11RenderTargetView* RenderTargetView = nullptr*/);
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
	void RemoveParticleSystem(int removeKey);
	void DeleteParticleSystem();//�S�폜

	//Z�o�b�t�@����
	void TurnOnZbuffer();
	void TurnOffZbuffer();

	//���z�֐�����
	virtual void OnNotify(Subject* subject_ = nullptr);

	//�A�N�Z�T
	void setParticleCounter(int set) { m_ParticleCounter = set; };
	std::map<int, ParticleSystem*> getParticleSystem() { return m_ParticleSystemDictionary; };
	int getParticleSystemNum() { return static_cast<int>(m_ParticleSystemDictionary.size()); };
	int getParticleSystemCount() { return m_ParticleCounter; };
};