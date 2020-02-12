#pragma once

#include	"ParticleSystemUtility.h"
//#include	"DX11util.h"

#include	<DirectXMath.h>
using namespace DirectX;
// 16�o�C�g�ɃA���C�������g����B
#define ALIGN16 _declspec(align(16))

class ParticleSystemSetTransform {
	ParticleSystemSetTransform() {}
public:
	ParticleSystemSetTransform(const ParticleSystemSetTransform&) = delete;
	ParticleSystemSetTransform& operator=(const ParticleSystemSetTransform&) = delete;
	ParticleSystemSetTransform(ParticleSystemSetTransform&&) = delete;
	ParticleSystemSetTransform& operator=(ParticleSystemSetTransform&&) = delete;

	static ParticleSystemSetTransform* GetInstance() {
		static ParticleSystemSetTransform instance;
		return &instance;
	}
	// �萔�o�b�t�@��`(���[���h�ϊ��s��)
	ALIGN16 struct ConstantBufferWorld
	{
		XMMATRIX World;				// ���[���h�ϊ��s��
	};
	// �萔�o�b�t�@��`(�r���[�ϊ��s��)
	ALIGN16 struct ConstantBufferView
	{
		XMMATRIX View;				// �r���[�ϊ��s��
	};
	// �萔�o�b�t�@��`(���t���[���ݒ�)
	ALIGN16 struct ConstantBufferProjection
	{
		XMMATRIX Projection;		// �v���W�F�N�V�����ϊ��s��
	};

	enum class TYPE {
		WORLD,
		VIEW,
		PROJECTION
	};

	void Uninit() {
		if (m_pConstantBufferWorld) {
			m_pConstantBufferWorld->Release();
			m_pConstantBufferWorld = nullptr;
		}
		if (m_pConstantBufferView) {
			m_pConstantBufferView->Release();
			m_pConstantBufferView = nullptr;
		}
		if (m_pConstantBufferProjection) {
			m_pConstantBufferProjection->Release();
			m_pConstantBufferProjection = nullptr;
		}
	}

	bool Init(ID3D11Device* device, ID3D11DeviceContext* devicecontext) {

		m_device = device;
		m_devicecontext = devicecontext;

		// �R���X�^���g�o�b�t�@�쐬
		bool sts = ParticleSystemUtility::CreateConstantBuffer(
			m_device,			// �f�o�C�X
			sizeof(ConstantBufferWorld),	// �T�C�Y
			&m_pConstantBufferWorld);			// �R���X�^���g�o�b�t�@�Q
		if (!sts) {
			MessageBox(NULL, "CreateBuffer(constant buffer world) error", "Error", MB_OK);
			return false;
		}

		// �R���X�^���g�o�b�t�@�쐬
		sts = ParticleSystemUtility::CreateConstantBuffer(
			m_device,			// �f�o�C�X
			sizeof(ConstantBufferView),	// �T�C�Y
			&m_pConstantBufferView);			// �R���X�^���g�o�b�t�@�Q
		if (!sts) {
			MessageBox(NULL, "CreateBuffer(constant buffer view) error", "Error", MB_OK);
			return false;
		}

		// �R���X�^���g�o�b�t�@�쐬
		sts = ParticleSystemUtility::CreateConstantBuffer(
			m_device,			// �f�o�C�X
			sizeof(ConstantBufferProjection),	// �T�C�Y
			&m_pConstantBufferProjection);			// �R���X�^���g�o�b�t�@�Q
		if (!sts) {
			MessageBox(NULL, "CreateBuffer(constant buffer projection) error", "Error", MB_OK);
			return false;
		}
		return true;
	}

	void SetTransform(TYPE type, DirectX::XMFLOAT4X4& matrix) {
		ALIGN16 XMMATRIX mat;
		mat = DirectX::XMLoadFloat4x4(&matrix);

		if (m_devicecontext != nullptr) {
			switch (type) {
			case TYPE::WORLD:
				m_CBWorld.World = XMMatrixTranspose(mat);
				m_devicecontext->UpdateSubresource(m_pConstantBufferWorld, 0, nullptr, &m_CBWorld, 0, 0);
				m_devicecontext->VSSetConstantBuffers(0, 1, &m_pConstantBufferWorld);
				m_devicecontext->PSSetConstantBuffers(0, 1, &m_pConstantBufferWorld);
				break;
			case TYPE::VIEW:
				m_CBView.View = XMMatrixTranspose(mat);
				m_devicecontext->UpdateSubresource(m_pConstantBufferView, 0, nullptr, &m_CBView, 0, 0);
				m_devicecontext->VSSetConstantBuffers(1, 1, &m_pConstantBufferView);
				m_devicecontext->PSSetConstantBuffers(1, 1, &m_pConstantBufferView);
				break;
			case TYPE::PROJECTION:
				m_CBProjection.Projection = XMMatrixTranspose(mat);
				m_devicecontext->UpdateSubresource(m_pConstantBufferProjection, 0, nullptr, &m_CBProjection, 0, 0);
				m_devicecontext->VSSetConstantBuffers(2, 1, &m_pConstantBufferProjection);
				m_devicecontext->PSSetConstantBuffers(2, 1, &m_pConstantBufferProjection);
				break;
			}
		}
	}
private:
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_devicecontext = nullptr;
	ID3D11Buffer * m_pConstantBufferWorld = nullptr;		// �R���X�^���g�o�b�t�@2
	ID3D11Buffer * m_pConstantBufferView = nullptr;			// �R���X�^���g�o�b�t�@2
	ID3D11Buffer * m_pConstantBufferProjection = nullptr;	// �R���X�^���g�o�b�t�@2
	ConstantBufferWorld m_CBWorld;				// b0 
	ConstantBufferView m_CBView;				// b1
	ConstantBufferProjection m_CBProjection;	// b2
};