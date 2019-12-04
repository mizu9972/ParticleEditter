//パーティクルシステムプログラム

//ヘッダー
#include <random>
#include "CParticle.h"
#include "dx11mathutil.h"
#include "DX11Settransform.h"

#define FPS 60
#define		SCREEN_X		1200
#define		SCREEN_Y		600
#ifndef ALIGN16
#define ALIGN16 _declspec(align(16))
#endif
/*------------------------
単位行列にする
--------------------------*/
void DX11MatrixIdentity_subDefiner(DirectX::XMFLOAT4X4& mat) {

	ALIGN16 XMMATRIX mtx;

	mtx = XMMatrixIdentity();

	XMStoreFloat4x4(&mat, mtx);
}

//メソッド
void ParticleSystem::Init(t_ParticleSystemState ParticleState_, const char* filename) {
	// ファイル名を
	m_TexName = filename;
	XMFLOAT4 col = { ParticleState_.m_Color[0],ParticleState_.m_Color[1],ParticleState_.m_Color[2],ParticleState_.m_Color[3] };
	m_texquad.Init(
		SCREEN_X / 2 + ParticleState_.m_PositionX,				// 左上Ｘ座標
		SCREEN_Y / 2 + -2 * ParticleState_.m_PositionY,		// 左上Ｙ座標
		ParticleState_.m_PositionZ,
		ParticleState_.m_Size,				// 幅
		ParticleState_.m_Size,				// 高さ
		col,		// 頂点カラー 
		m_TexName);			// ファイル名

	SetParticleSystemState(ParticleState_);
	Start();
}

void ParticleSystem::ZeroInit() {
	if (m_ParticleStateZero != NULL) {

		SetParticleSystemState(*m_ParticleStateZero);

		Start();
	}

}


void ParticleSystem::Start() {
	//パーティクル処理開始
	//パーティクル生成
	if (Particles != NULL) {
		delete[] Particles;
	}
	Particles = new m_Particles[m_ParticleState.m_ParticleNum];
	isSystemActive = true;
	ParticlesDeathCount = 0;

	m_SystemLifeTime = m_ParticleState.m_DuaringTime;
	NowTime = CTimer::GetInstance()->GetActuallyTimePoint();//開始時間設定
	//パーティクル設定-------------------------------------------------------------------------
	srand((int)NowTime);
	
	XMFLOAT3 SetRandAngle;
	XMFLOAT4 qt;//クォータニオン
	XMFLOAT4 axisX, axisY, axisZ;//ベクトル保存
	XMFLOAT4 qtx, qty, qtz;//クォータニオン
	XMFLOAT4 tempqt1, tempqt2, tempqt3;//一時保存クォータニオン
	m_Particles* p_PickParticle;//操作するパーティクルのポインタ保存用

	//パーティクルの初期設定する
	for (int ParticlesNum = 0; ParticlesNum < m_ParticleState.m_ParticleNum; ParticlesNum++) {

		p_PickParticle = &Particles[ParticlesNum];

		DX11MatrixIdentity_subDefiner(p_PickParticle->Matrix);
		//ランダムな角度をそれぞれ設定
		SetRandAngle.x = (float)(rand() % m_ParticleState.m_AngleRangeX);
		SetRandAngle.y = (float)(rand() % m_ParticleState.m_AngleRangeY);
		SetRandAngle.z = (float)(rand() % m_ParticleState.m_AngleRangeZ);

		//角度を取り出し-------------------------------
		axisX.x = p_PickParticle->Matrix._11;
		axisX.y = p_PickParticle->Matrix._12;
		axisX.z = p_PickParticle->Matrix._13;

		axisY.x = p_PickParticle->Matrix._21;
		axisY.y = p_PickParticle->Matrix._22;
		axisY.z = p_PickParticle->Matrix._23;

		axisZ.x = p_PickParticle->Matrix._31;
		axisZ.y = p_PickParticle->Matrix._32;
		axisZ.z = p_PickParticle->Matrix._33;
		//-------------------------------------------

		//行列からクォータニオンを生成
		DX11GetQtfromMatrix(p_PickParticle->Matrix, qt);


		//指定軸回転のクォータニオンを生成
		DX11QtRotationAxis(qtx, axisX, SetRandAngle.x + m_ParticleState.m_AngleX);
		DX11QtRotationAxis(qty, axisY, SetRandAngle.y + m_ParticleState.m_AngleY);
		DX11QtRotationAxis(qtz, axisZ, SetRandAngle.z + m_ParticleState.m_AngleZ);

		//クォータニオンを正規化

		//クォータニオンを合成

		DX11QtMul(tempqt1, qt, qtx);

		DX11QtMul(tempqt2, qty, qtz);

		DX11QtMul(tempqt3, tempqt1, tempqt2);

		//クォータニオンをノーマライズ
		DX11QtNormalize(tempqt3, tempqt3);

		DX11MtxFromQt(p_PickParticle->Matrix, tempqt3);

		p_PickParticle->Matrix._41 = m_ParticleState.m_PositionX;
		p_PickParticle->Matrix._42 = m_ParticleState.m_PositionY;
		p_PickParticle->Matrix._43 = m_ParticleState.m_PositionZ;

		//発生までの待機時間設定
		//パーティクルの発生時間をパーティクル数で等分し、それぞれ割り当てる
		p_PickParticle->DelayTime = m_ParticleState.m_DuaringTime / m_ParticleState.m_ParticleNum * ParticlesNum;
		//その他設定
		p_PickParticle->LifeTime = m_ParticleState.m_MaxLifeTime;

		//待機中に
		p_PickParticle->isAlive = false;
		p_PickParticle->isWaiting = true;
	}
	//--------------------------------------------------------------------------------------
}

void ParticleSystem::Update() {
	//更新処理
	float ParticleSize;//サイズ変化用
	m_Particles* p_PickParticle;//操作するパーティクルのポインタ保存用


	if (isSystemActive == false) {
		return;
	}

	float Speed_ = m_ParticleState.m_Speed;
	NowTime = CTimer::GetInstance()->GetDurationTime(NowTime) / 1e+9f;//経過時間取得
	
	for (int ParticlesNum = 0; ParticlesNum < m_ParticleState.m_ParticleNum; ParticlesNum++) {

		p_PickParticle = &Particles[ParticlesNum];

		//待機中なら待機時間減少
		if (p_PickParticle->isWaiting == true) {
			p_PickParticle->DelayTime -= NowTime;

			//待機時間が0以下なら活性化
			if (p_PickParticle->DelayTime <= 0) {
				p_PickParticle->isWaiting = false;
				p_PickParticle->isAlive = true;
			}
		}

		if (p_PickParticle->isAlive == false) {
			continue;
		}
		//更新処理-------------------------------------------------------------------
		if (p_PickParticle->Matrix._43 != 0.0f) {
			ParticleSize = m_ParticleState.m_Size / p_PickParticle->Matrix._43;
		}
		else {
			ParticleSize = 1.0f;
		}
		
		m_ParticleState.m_Size += ParticleSize;

		//m_texquad.SetSize(m_ParticleState.m_Size, m_ParticleState.m_Size, m_ParticleState.m_Size);
		//RefParticle();

		//速度分移動させる
		p_PickParticle->Matrix._41 += p_PickParticle->Matrix._31 * Speed_ * NowTime;
		p_PickParticle->Matrix._42 += p_PickParticle->Matrix._32 * Speed_ * NowTime;
		p_PickParticle->Matrix._43 += p_PickParticle->Matrix._33 * Speed_ * NowTime;

		//生存時間減少
		p_PickParticle->LifeTime -= NowTime;


		//--------------------------------------------------------------------------

		//死亡判定
		if (p_PickParticle->LifeTime <= 0) {
			p_PickParticle->isAlive = false;
			ParticlesDeathCount += 1;
		}
	}
	m_SystemLifeTime -= NowTime;

	
	//パーティクル終了判定
	if (m_SystemLifeTime <= 0) {
		isSystemActive = false;
		if (m_ParticleState.isLooping == true) {//ループ
			Start();
		}
	}
}

void ParticleSystem::Draw() {
	if (isSystemActive == false) {
		return;
	}

	for (int ParticlesNum = 0; ParticlesNum < m_ParticleState.m_ParticleNum; ParticlesNum++) {
		if (Particles[ParticlesNum].isAlive == false) {
			continue;
		}

		// 描画
		m_texquad.SetPos(Particles[ParticlesNum].Matrix._41, Particles[ParticlesNum].Matrix._42, Particles[ParticlesNum].Matrix._43);
		m_texquad.Draw();
	}

}

void ParticleSystem::UnInit() {
	if (Particles != NULL) {
		delete[] Particles;
	}
}

bool ParticleSystem::FInState(const char* FileName_) {
	//パーティクルシステムの情報をファイルから読み込む

	//カレントディレクトリを操作する
	TCHAR crDir[MAX_PATH + 1];
	t_ParticleSystemState GetState;


	GetCurrentDirectory(MAX_PATH + 1, crDir);//操作前のディレクトリを取得
	SetCurrentDirectory(".\\InPutData");//読み込み先フォルダへカレントディレクトリを変更

	//ファイル読み込み
	FILE *Fp;
	errno_t ErrorCheck;
	try {
		ErrorCheck = fopen_s(&Fp, FileName_, "rb");//

		SetCurrentDirectory(crDir);//カレントディレクトリを元に戻す

		if (ErrorCheck != 0) {
			throw 1;
		}
		fread(&GetState, sizeof(t_ParticleSystemState), 1, Fp);//データ読み込み
		
		SetParticleSystemState(GetState);//情報セット

		fclose(Fp);
	}
	catch (int i) {//ファイル読み込み失敗したら
		return i;
	};

	Init(m_ParticleState, "assets/ParticleTexture/DefaultParticle.png");
	return true;
}

bool ParticleSystem::FInTex(const char* FileName_) {
	//テクスチャ読み込み
	std::string Texname = ".\\InPutData/";
	Texname += FileName_;//ファイルの位置を指定

	Init(m_ParticleState, Texname.c_str());//指定したファイルを利用して初期化

	return 0;
}

void ParticleSystem::FOutState() {
	//パーティクルシステムの情報をファイルへ書き出し

	//カレントディレクトリを操作する
	TCHAR crDir[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH + 1, crDir);//操作前のディレクトリを取得
	SetCurrentDirectory(".\\OutPutData");//書き出し先のフォルダへカレントディレクトリを変更

	//ファイル書き出し
	FILE *Fp;

	fopen_s(&Fp, "ParticleState.txt", "wb");

	fwrite(&m_ParticleState, sizeof(t_ParticleSystemState), 1, Fp);

	fclose(Fp);

	SetCurrentDirectory(crDir);//カレントディレクトリを元に戻す
}

//アクセス
void ParticleSystem::SetParticleSystemState(t_ParticleSystemState SetParticleSystemState_) {
	memcpy(&m_ParticleState, &SetParticleSystemState_, sizeof(t_ParticleSystemState));
}

void ParticleSystem::RefParticle() {
	m_texquad.ChangePosSize(SCREEN_X / 2 + m_ParticleState.m_PositionX,				// 左上Ｘ座標
		SCREEN_Y / 2 + -2 * m_ParticleState.m_PositionY,
		m_ParticleState.m_PositionZ,
		m_ParticleState.m_Size, 
		m_ParticleState.m_Size);
}