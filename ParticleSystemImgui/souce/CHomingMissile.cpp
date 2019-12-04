#include "CHomingMissile.h"

const float MissileSpeed = 3.0f;

//------------------------------------
//��̃x�N�g������N�H�[�^�j�I���𐶐�����
//------------------------------------
XMFLOAT4 CHomingMissile::RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d) {
	
	XMFLOAT3 Axis;//��
	XMFLOAT4 q; //�N�H�[�^�j�I��

	//���K��(�O���Ő��K���ς�)
	//D2DXVec3Normalizze(&v0,&v0);
	//D2DXVec3Normalizze(&v1,&v1);

	DX11Vec3Cross(Axis, v0, v1);

	DX11Vec3Dot(d, v0, v1);
	//�^�[�Q�b�g�̕����Ǝ��@���قƂ�ǈ�v�����Ƃ��A���ς̒l���P�𒴂���(-1�������)��������̂ŕ␳����
	if (d > 1.0) {
		d = 1.0;
	}
	if (d <= -1) {
		d = -1.0f;
	}

	float s = (float)sqrt((1 + d) * 2);
	if (s == 0.0f) {
		DX11QtIdentity(q);//�^�[�Q�b�g��ǂ��z����
	}
	else {
		q.x = Axis.x / s;
		q.y = Axis.y / s;
		q.z = Axis.z / s;
		q.w = s / 2;
	}

	return q;
}

//--------------------------------------------
//�~�T�C���̃f�[�^���X�V����
//�^�[�Q�b�g�Ƃ̋�����lng��菬�����Ȃ�����true��Ԃ�
//--------------------------------------------
bool CHomingMissile::Update(XMFLOAT3 TargetPosition, float l) {
	
	bool sts = false;
	float lng;

	XMFLOAT4 TargetQt;//�^�[�Q�b�g�����̎p��

	XMFLOAT3 TargetVector;

	//���݈ʒu����^�[�Q�b�g�����ւ̃x�N�g�������߂�
	TargetVector.x = TargetPosition.x - m_Position.x;
	TargetVector.x = TargetPosition.y - m_Position.y;
	TargetVector.x = TargetPosition.z - m_Position.z;

	DX11Vec3Length(TargetVector, lng);
	if (lng <= l) {
		return true;
	}

	XMFLOAT3 ZDir = m_Velocity;//�~�T�C���̕����x�N�g��(Z����)

	//���K��
	DX11Vec3Normalize(TargetVector, TargetVector);
	DX11Vec3Normalize(ZDir, ZDir);

	float Dot;//�Q�{�̃x�N�g���̓��ϒl
	TargetQt = RotationArc(ZDir, TargetVector, Dot);//�Q�{�̃x�N�g������ׂ��p�x�ƃN�H�[�^�j�I�������߂�
	float AngleDiff = acosf(Dot);//���W�A���p�x

	//�~�T�C���̎p�������肷��
	if (m_AddRotMax >= AngleDiff) {
		//�p�x�̍����X�V�ł���p�x���傫������������
		DX11QtMul(m_Quaternion, m_Quaternion, TargetQt);
	}
	else {
		float t = m_AddRotMax / AngleDiff;

		XMFLOAT4 toqt;
		DX11QtMul(toqt, m_Quaternion, TargetQt);
		DX11QtSlerp(m_Quaternion, toqt, t, m_Quaternion);
	}

	//�N�H�[�^�j�I�����s��ɂ���
	//���݂̎p�����N�H�[�^�j�I���ɂ���
	DX11MtxFromQt(m_Matrix, m_Quaternion);

	//�ʒu���X�V	
	m_Position.x += m_Velocity.x;
	m_Position.y += m_Velocity.y;
	m_Position.z += m_Velocity.z;

	//�s��ɃZ�b�g
	m_Matrix._41 = m_Position.x;
	m_Matrix._42 = m_Position.y;
	m_Matrix._43 = m_Position.z;

	return false;
}