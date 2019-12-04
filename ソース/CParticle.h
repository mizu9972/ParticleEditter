#pragma once
#include	<directxmath.h>
#include	<string.h>

#include "CTimer.h"
#include "C2DQuadtex.h"

using namespace DirectX;

//パーティクルシステム設定構造体
//手動で設定して割り当てる
typedef struct {
	float m_PositionX;//座標
	float m_PositionY;
	float m_PositionZ;

	int m_AngleX;
	int m_AngleY;
	int m_AngleZ;

	int m_AngleRangeX;//発生させる角度の範囲
	int m_AngleRangeY;
	int m_AngleRangeZ;

	float m_DuaringTime;//発生時間
	float m_Size;//大きさ
	float m_MaxLifeTime;//最長生存時間
	float m_Speed;//速度
	int m_ParticleNum;//生成するパーティクル個数

	float m_Color[4];//パーティクルの色
	bool isLooping;
}t_ParticleSystemState;

class ParticleSystem {
private:
	//生成するパーティクルの情報
	//自動で設定される
	struct m_Particles {
		float DelayTime;//発射待機時間
		float LifeTime;//生存時間
		XMFLOAT4X4 Matrix;//行列
		bool isAlive;//生存しているか
		bool isWaiting;//発生待機中か
	};
	float NowTime;
	C2DQuadTex m_texquad;
	const char* m_TexName;
protected:
	t_ParticleSystemState* m_ParticleStateZero = NULL;//初期値保存用
	t_ParticleSystemState m_ParticleState;//パーティクルシステム設定
	m_Particles* Particles = NULL;
	int ParticlesDeathCount;
	float m_SystemLifeTime;//パーティクルシステム残り時間
	bool isSystemActive = false;
public:
	ParticleSystem() {};
	ParticleSystem(t_ParticleSystemState ParticleState_) {
		memcpy(m_ParticleStateZero, &ParticleState_, sizeof(t_ParticleSystemState));
	};
	~ParticleSystem() {
		if (Particles != NULL) {
			Particles = NULL;
			delete[] Particles;
		}
	};

	//基本処理メソッド
	void Init(t_ParticleSystemState ParticleState_, const char* filename);//初期化
	void ZeroInit();//コンストラクタで数値設定した場合、生成時の状態に初期化できる (デフォルトに戻す処理(未実装)用)
	void Start();//パーティクル発生開始
	void Update();
	void Draw();
	void UnInit();
	//ファイル入出力メソッド
	bool FInState(const char* FileName_);
	void FOutState();
	bool FInTex(const char* FileName_);

	//アクセスメソッド
	//setter
	void SetSize(float Size_);
	void SetLifeTime(float LifeTime_);
	void SetSpeed(float Speed_);
	void SetFileName(const char* FileName_);
	void SetParticleSystemState(t_ParticleSystemState SetState_);

private:
	void RefParticle();//パーティクルの情報更新
};