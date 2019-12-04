#pragma once

#include <directxmath.h>
#include "dx11mathutil.h"

using namespace DirectX;
class CHomingMissile {
private:
	XMFLOAT3 m_Position; //�~�T�C���̌��݈ʒu
	XMFLOAT3 m_Velocity; //�~�T�C���̌��ݑ��x
	XMFLOAT4X4 m_Matrix; //�s��
	XMFLOAT4 m_Quaternion; //�N�H�[�^�j�I��
	float m_AddRotMax; //�ő��]���x

	XMFLOAT4 RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d);//v0�x�N�g����v1�x�N�g���ɏd�ˍ��킹��N�H�[�^�j�I�������߂�

public:
	//�R���X�g���N�^
	CHomingMissile(const XMFLOAT3& Position, 
		const XMFLOAT3& Velocity,
		const XMFLOAT4X4& mat, 
		float MaxAngle) :m_AddRotMax(XM_PI * MaxAngle / 180.0f) {

		//���������Z�b�g
		SetInitPosition(Position);
		SetInitVelocity(Velocity);
		DX11GetQtfromMatrix(mat, m_Quaternion);
		m_Matrix = mat;
	}
	CHomingMissile(const XMFLOAT4X4& mat, float MaxAngle) :m_AddRotMax(XM_PI * MaxAngle / 180.0f) {
		m_Position.x = mat._41;
		m_Position.y = mat._42;
		m_Position.z = mat._43;
		
		m_Velocity.x = mat._31;
		m_Velocity.y = mat._32;
		m_Velocity.z = mat._33;
		DX11GetQtfromMatrix(mat, m_Quaternion);
		m_Matrix = mat;
	}

	//�f�X�g���N�^
	virtual ~CHomingMissile() {};

	//�A�N�Z�X���\�b�h
	//�������x���Z�b�g����
	void SetInitVelocity(XMFLOAT3 NowVelocity) {
		m_Velocity = NowVelocity;
	}

	//�����ʒu���Z�b�g����
	void SetInitPosition(XMFLOAT3 InitPos) {
		m_Position = InitPos;
	}

	//�p���ƈʒu��\���s���Ԃ�
	const XMFLOAT4X4& GetMatrix() const {
		return m_Matrix;
	}

	bool Update(const XMFLOAT3 TargetPosition, float lng);
};