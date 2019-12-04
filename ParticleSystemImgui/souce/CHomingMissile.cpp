#include "CHomingMissile.h"

const float MissileSpeed = 3.0f;

//------------------------------------
//二つのベクトルからクォータニオンを生成する
//------------------------------------
XMFLOAT4 CHomingMissile::RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d) {
	
	XMFLOAT3 Axis;//軸
	XMFLOAT4 q; //クォータニオン

	//正規化(外部で正規化済み)
	//D2DXVec3Normalizze(&v0,&v0);
	//D2DXVec3Normalizze(&v1,&v1);

	DX11Vec3Cross(Axis, v0, v1);

	DX11Vec3Dot(d, v0, v1);
	//ターゲットの方向と自機がほとんど一致したとき、内積の値が１を超える(-1を下回る)時があるので補正する
	if (d > 1.0) {
		d = 1.0;
	}
	if (d <= -1) {
		d = -1.0f;
	}

	float s = (float)sqrt((1 + d) * 2);
	if (s == 0.0f) {
		DX11QtIdentity(q);//ターゲットを追い越した
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
//ミサイルのデータを更新する
//ターゲットとの距離がlngより小さくなったらtrueを返す
//--------------------------------------------
bool CHomingMissile::Update(XMFLOAT3 TargetPosition, float l) {
	
	bool sts = false;
	float lng;

	XMFLOAT4 TargetQt;//ターゲット方向の姿勢

	XMFLOAT3 TargetVector;

	//現在位置からターゲット方向へのベクトルを求める
	TargetVector.x = TargetPosition.x - m_Position.x;
	TargetVector.x = TargetPosition.y - m_Position.y;
	TargetVector.x = TargetPosition.z - m_Position.z;

	DX11Vec3Length(TargetVector, lng);
	if (lng <= l) {
		return true;
	}

	XMFLOAT3 ZDir = m_Velocity;//ミサイルの方向ベクトル(Z方向)

	//正規化
	DX11Vec3Normalize(TargetVector, TargetVector);
	DX11Vec3Normalize(ZDir, ZDir);

	float Dot;//２本のベクトルの内積値
	TargetQt = RotationArc(ZDir, TargetVector, Dot);//２本のベクトルから為す角度とクォータニオンを求める
	float AngleDiff = acosf(Dot);//ラジアン角度

	//ミサイルの姿勢を決定する
	if (m_AddRotMax >= AngleDiff) {
		//角度の差が更新できる角度より大きいか小さいか
		DX11QtMul(m_Quaternion, m_Quaternion, TargetQt);
	}
	else {
		float t = m_AddRotMax / AngleDiff;

		XMFLOAT4 toqt;
		DX11QtMul(toqt, m_Quaternion, TargetQt);
		DX11QtSlerp(m_Quaternion, toqt, t, m_Quaternion);
	}

	//クォータニオンを行列にする
	//現在の姿勢をクォータニオンにする
	DX11MtxFromQt(m_Matrix, m_Quaternion);

	//位置を更新	
	m_Position.x += m_Velocity.x;
	m_Position.y += m_Velocity.y;
	m_Position.z += m_Velocity.z;

	//行列にセット
	m_Matrix._41 = m_Position.x;
	m_Matrix._42 = m_Position.y;
	m_Matrix._43 = m_Position.z;

	return false;
}