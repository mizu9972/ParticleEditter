#pragma once

#include <directxmath.h>
#include "dx11mathutil.h"

using namespace DirectX;
class CHomingMissile {
private:
	XMFLOAT3 m_Position; //ミサイルの現在位置
	XMFLOAT3 m_Velocity; //ミサイルの現在速度
	XMFLOAT4X4 m_Matrix; //行列
	XMFLOAT4 m_Quaternion; //クォータニオン
	float m_AddRotMax; //最大回転速度

	XMFLOAT4 RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d);//v0ベクトルをv1ベクトルに重ね合わせるクォータニオンを求める

public:
	//コンストラクタ
	CHomingMissile(const XMFLOAT3& Position, 
		const XMFLOAT3& Velocity,
		const XMFLOAT4X4& mat, 
		float MaxAngle) :m_AddRotMax(XM_PI * MaxAngle / 180.0f) {

		//初期情報をセット
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

	//デストラクタ
	virtual ~CHomingMissile() {};

	//アクセスメソッド
	//初期速度をセットする
	void SetInitVelocity(XMFLOAT3 NowVelocity) {
		m_Velocity = NowVelocity;
	}

	//初期位置をセットする
	void SetInitPosition(XMFLOAT3 InitPos) {
		m_Position = InitPos;
	}

	//姿勢と位置を表す行列を返す
	const XMFLOAT4X4& GetMatrix() const {
		return m_Matrix;
	}

	bool Update(const XMFLOAT3 TargetPosition, float lng);
};