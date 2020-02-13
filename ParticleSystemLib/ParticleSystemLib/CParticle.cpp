//パーティクルシステムプログラム

//ヘッダー
#include "stdafx.h"

#include <random>
#include <algorithm>
#include "CParticle.h"
#include "ParticleMathUtil.h"
#include "ParticleSystemUtility.h"

#define		SCREEN_X		1200
#define		SCREEN_Y		600
#define		PARTICLE_NUM_PER_THREAD (64) //csParticle.hlsl内と同じ定数を宣言
#define		THREAD_NUM (256)

#ifndef     ALIGN16
#define     ALIGN16 _declspec(align(16))
#endif

#define STS_ifERROR_FUNCTION(x) if (x != true) MessageBox(NULL, "AnyFunction is Error", "Error", MB_OK)


constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/particle.png";
constexpr auto PARTICLE_PS_SHADER = "shader/psParticle.fx";
constexpr auto PARTICLE_VS_SHADER = "shader/vsPrticle.fx";

constexpr auto SOFTPARTICLE_PS_SHADER = "shader/psSoftParticle.fx";
constexpr auto SOFTPARTICLE_VS_SHADER = "shader/vsSoftParticle.fx";
/*------------------------
単位行列にする
--------------------------*/
void DX11MatrixIdentity_subDefiner(DirectX::XMFLOAT4X4& mat) {

	ALIGN16 XMMATRIX mtx;

	mtx = XMMatrixIdentity();

	XMStoreFloat4x4(&mat, mtx);
}

//メソッド

//関数ポインタ利用して切り替わる処理を呼び出すメソッド---------------------------
//パーティクル処理開始
void ParticleSystem::Start() {
	if (isEmitting == true && m_ParticleState.isEmitting == true) {
		return;
	}
	isEmitting = m_ParticleState.isEmitting;
	(this->*fpStartFunc)();
}

//更新処理
void ParticleSystem::Update() {
	if (isUpdateActive == false) {
		return;
	}
	(this->*fpUpdateFunc)();
}

//描画処理
void ParticleSystem::Draw(const XMFLOAT4X4& CameraMatrix) {
	if (isDrawActive == false) {
		return;
	}

	(this->*fpDrawFunc)(CameraMatrix);

}
//--------------------------------------------------------------------

//初期化
ParticleSystem& ParticleSystem::Init(ID3D11Device* device, ID3D11DeviceContext* devicecontext,t_ParticleSystemState* ParticleState_, const char* filename) {
	//パーティクル初期化

	m_Device = device;
	m_DeviceContext = devicecontext;
	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
	}

	t_ParticleSystemState newState;
	//引数のステータスを設定
	if (ParticleState_ != nullptr) {
		newState = *ParticleState_;

	}
	SetParticleSystemState(&newState);
	InitComputeShader();

	if (filename != nullptr) {
		// ファイル名を保存
		strcpy_s(m_ParticleState.m_TextureName, filename);
	}

	ChangeSoftParticleMode(newState.isSoftParticle);

	bool sts = ParticleSystemUtility::CreateConstantBuffer(m_Device, sizeof(ConstantBufferSoftParticle), m_CpCBufferSoftParticle.GetAddressOf());
	STS_ifERROR_FUNCTION(sts);
	SetSoftPConstantBuffer();
	
	
	//ビルボード初期化---------------------------------------------------------------------------------------------------------------
	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
	m_BillBoard.SetUV(u, v);
	m_BillBoard.LoadTexTure(m_ParticleState.m_TextureName);
	//----------------------------------------------------------------------------------------------------------------------------

	Start();
	return *this;
}

//ステータス初期化
void ParticleSystem::ZeroInit() {

	if (m_ParticleStateZero != NULL) {

		SetParticleSystemState(m_ParticleStateZero);

		Start();
	}

}

//コンピュートシェーダー初期化
void ParticleSystem::InitComputeShader() {

	//Comポインタ設定
	m_CpSRV.Attach(m_pSRV);
	m_CpUAV.Attach(m_pUAV);
	m_CpResult.Attach(m_pResult);
	m_CpCBufferSoftParticle.Attach(m_ConstantBufferSoftParticle);
	m_CpGetBuf.Attach(getbuf);
	m_CpBuf.Attach(m_pbuf);
}

void ParticleSystem::ParticleDetalInit() {
	//パーティクル群の初期化
	m_Particles setState;

	//初期値設定
	setState.CountTime  = 0;
	setState.isWaiting  = true;
	setState.isAlive    = false;
	setState.LifeTime   = m_ParticleState.m_MaxLifeTime;
	setState.Matrix._41 = m_ParticleState.m_Position[0];
	setState.Matrix._42 = m_ParticleState.m_Position[1];
	setState.Matrix._43 = m_ParticleState.m_Position[2];
	setState.ZAngle     = rand() % 360;

	m_ParticleDetails.clear();
	for (int ParticleNum = 0; ParticleNum < m_ParticleState.m_ParticleMax; ParticleNum++) {
		m_ParticleDetails.emplace_back(setState);
	}
}

//GPUパーティクルシステム更新
void ParticleSystem::UpdateComputeShader() {

	if (m_ParticleNum <= 0) {
		return;
	}
	NowTime = 1.0f / FPS;
	m_SystemLifeTime -= NowTime;
	//パーティクル終了判定
	if (isSystemActive == true) {
		if (m_SystemLifeTime <= 0) {
			if (m_NextParticleNumberVector.size() > 0) {
				//次のパーティクル開始
				Notify(this);
			}
			if(m_ParticleState.isLooping == false){
				isSystemActive = false;
			}
			m_SystemLifeTime = m_ParticleState.m_DuaringTime + m_ParticleState.m_StartDelayTime;
		}
	}

	//コンピュートシェーダーを実行
	const UINT dispatchX = UINT(ceil(float(m_ParticleNum) / float(THREAD_NUM * PARTICLE_NUM_PER_THREAD)));
	ParticleSystemUtility::RunComputeShader(m_DeviceContext, m_ComputeShader, 1, m_CpSRV.GetAddressOf(), m_CpUAV.Get(), dispatchX, 1, 1);

	//データ受け取り
	m_DeviceContext->CopyResource(m_CpGetBuf.Get() , m_CpResult.Get());//バッファコピー
	m_DeviceContext->Map(m_CpGetBuf.Get(), 0, D3D11_MAP_READ, 0, &m_MappedSubResource);

	OutState = reinterpret_cast<m_ParticleUAVState*>(m_MappedSubResource.pData);//データ獲得

	m_DeviceContext->Unmap(m_CpGetBuf.Get(), 0);
}

void ParticleSystem::UpdateNomal() {

	XMFLOAT4 TargetQt;//ターゲット方向の姿勢
	XMFLOAT3 TargetVector;

	float Speed_ = m_ParticleState.m_Speed;

	NowTime = 1.0f / FPS;
	if (m_SystemLifeTime >= 0) {
		m_SystemLifeTime -= NowTime;
	}
	m_SpownTimeCount += NowTime;

	//パーティクル更新処理
	{
		for (int ParticleNum = 0; ParticleNum < m_ParticleDetails.size();ParticleNum++) {
			if (m_ParticleDetails[ParticleNum].isAlive == false) {
				continue;
			}
			
			//ホーミング処理
			if (m_ParticleState.isChaser == true) {

				//座標保存
				XMFLOAT3 ParticlePosition = XMFLOAT3(m_ParticleDetails[ParticleNum].Matrix._41, m_ParticleDetails[ParticleNum].Matrix._42, m_ParticleDetails[ParticleNum].Matrix._43);
				
				//現在位置からターゲット方向へのベクトルを求める
				TargetVector.x = m_TargetPos.x - m_ParticleDetails[ParticleNum].Matrix._41;
				TargetVector.y = m_TargetPos.y - m_ParticleDetails[ParticleNum].Matrix._42;
				TargetVector.z = m_TargetPos.z - m_ParticleDetails[ParticleNum].Matrix._43;

				//Z方向ベクトル
				XMFLOAT3 ZDir = XMFLOAT3(m_ParticleDetails[ParticleNum].Matrix._31, m_ParticleDetails[ParticleNum].Matrix._32, m_ParticleDetails[ParticleNum].Matrix._33);

				//正規化
				ParticleSystemMathUtil::DX11Vec3Normalize(TargetVector, TargetVector);
				ParticleSystemMathUtil::DX11Vec3Normalize(ZDir, ZDir);
				ParticleSystemMathUtil::DX11GetQtfromMatrix(m_ParticleDetails[ParticleNum].Matrix, m_Quaternion);

				float Dot;//２本のベクトルの内積値
				TargetQt = RotationArc(ZDir, TargetVector, Dot);//２本のベクトルから為す角度とクォータニオンを求める
				float AngleDiff = acosf(Dot);//ラジアン角度
				float AngleMax = XM_PI * m_ParticleState.m_MaxChaseAngle / 180.0f;
				float AngleMin = XM_PI * m_ParticleState.m_MinChaseAngle / 180.0f;

				//パーティクルの姿勢を決定する
				if (AngleMin > AngleDiff) {
					//何もしない
				}
				else if (AngleMax >= AngleDiff) {
					//角度の差が更新できる角度より大きいか小さいか
					ParticleSystemMathUtil::DX11QtMul(m_Quaternion, m_Quaternion, TargetQt);
				}
				else {
					float t = AngleMax / AngleDiff;

					XMFLOAT4 toqt;
					ParticleSystemMathUtil::DX11QtMul(toqt, m_Quaternion, TargetQt);
					ParticleSystemMathUtil::DX11QtSlerp(m_Quaternion, toqt, t, m_Quaternion);
				}

				//クォータニオンを行列にする
				//現在の姿勢をクォータニオンにする
				ParticleSystemMathUtil::DX11MtxFromQt(m_ParticleDetails[ParticleNum].Matrix, m_Quaternion);

				m_ParticleDetails[ParticleNum].Matrix._41 = ParticlePosition.x;
				m_ParticleDetails[ParticleNum].Matrix._42 = ParticlePosition.y;
				m_ParticleDetails[ParticleNum].Matrix._43 = ParticlePosition.z;
			}
			
			m_ParticleDetails[ParticleNum].CountTime += NowTime;

			//加速度計算
			Speed_ = m_ParticleState.m_Speed + m_ParticleState.m_Accel * m_ParticleDetails[ParticleNum].CountTime * m_ParticleDetails[ParticleNum].CountTime;
			if (m_ParticleState.m_Accel != 0.0f) {
				Speed_ = min(Speed_, m_ParticleState.m_MaxSpeed);
				Speed_ = max(Speed_, m_ParticleState.m_MinSpeed);
			}

			//速度分移動させる
			m_ParticleDetails[ParticleNum].Matrix._41 += m_ParticleDetails[ParticleNum].Matrix._31 * Speed_ * NowTime;
			m_ParticleDetails[ParticleNum].Matrix._42 += m_ParticleDetails[ParticleNum].Matrix._32 * Speed_ * NowTime;
			m_ParticleDetails[ParticleNum].Matrix._43 += m_ParticleDetails[ParticleNum].Matrix._33 * Speed_ * NowTime;

			if (m_ParticleState.UseGravity) {
				//重力
				m_ParticleDetails[ParticleNum].Matrix._41 += m_ParticleState.m_Gravity[0] / 100.0f * m_ParticleDetails[ParticleNum].CountTime;
				m_ParticleDetails[ParticleNum].Matrix._42 += m_ParticleState.m_Gravity[1] / 100.0f * m_ParticleDetails[ParticleNum].CountTime;
				m_ParticleDetails[ParticleNum].Matrix._43 += m_ParticleState.m_Gravity[2] / 100.0f * m_ParticleDetails[ParticleNum].CountTime;
			}

			//生存時間減少
			m_ParticleDetails[ParticleNum].LifeTime -= NowTime;

			//回転
			m_ParticleDetails[ParticleNum].ZAngle += m_ParticleState.m_RotateSpeed;
			m_ParticleDetails[ParticleNum].ZAngle = m_ParticleDetails[ParticleNum].ZAngle % 360;

			//死亡判定
			if (m_ParticleDetails[ParticleNum].LifeTime <= 0) {
				m_ParticleDetails[ParticleNum].isAlive = false;
				m_ParticleDetails[ParticleNum].isWaiting = true;
				ParticlesDeathCount += 1;
			}
		}
	}

	//パーティクル発生処理
	{
		float SpownTimePerParticle = 1.0f / m_ParticleState.m_ParticleSpownSpeed;
		//一定時間経過で待機状態のパーティクルを発生させる
		if (isSystemActive == true) {
			if (m_SpownTimeCount > SpownTimePerParticle) {

				if (m_ParticleDetails.size() > 0) {
					//複数パーティクル同時発生させる可能性もあるので
					//発生個数を計算しループさせる
					for (int Num = 0; Num < m_SpownTimeCount / SpownTimePerParticle; Num++) {
						//パーティクル発生処理
						if (m_ParticleDetails[m_Iter].isWaiting == true) {
							//生存パーティクル追加
							AddParticle(&m_ParticleDetails[m_Iter]);
						}
						m_Iter += 1;

						if (m_Iter >= m_ParticleDetails.size()) {
							m_Iter = 0;
						}
					}
				}

				m_SpownTimeCount = 0;
			}
		}
	}

	//パーティクル終了判定
	if (m_SystemLifeTime <= 0) {
		if (isSystemActive != true) {
			return;
		}
		if (m_NextParticleNumberVector.size() > 0) {
			//次のパーティクル開始
			Notify(this);
		}

		if (m_ParticleState.isLooping == false) {
			//ループしないならパーティクル発生停止
			isSystemActive = false;

		}
		else {
			//再スタート
			m_SystemLifeTime = m_ParticleState.m_DuaringTime + m_ParticleState.m_StartDelayTime;
		}

	}
}

void ParticleSystem::UpdateSRV(){

	//入力用バッファを更新
	InState.iPosition       = { m_ParticleState.m_Position[0],m_ParticleState.m_Position[1],m_ParticleState.m_Position[2],0 };
	InState.iAngle          = { 360 - m_ParticleState.m_Angle[0],360 - m_ParticleState.m_Angle[1],360 - m_ParticleState.m_Angle[2],0 };
	InState.iAngleRange     = m_ParticleState.m_AngleRange;
	InState.iDuaringTime    = m_ParticleState.m_DuaringTime;
	InState.iDelayTime      = m_ParticleState.m_StartDelayTime;
	InState.iMaxLifeTime    = m_ParticleState.m_MaxLifeTime;
	InState.iSpeed          = m_ParticleState.m_Speed;
	InState.iAccel          = m_ParticleState.m_Accel;
	InState.iMinSpeed       = m_ParticleState.m_MinSpeed;
	InState.iMaxSpeed       = m_ParticleState.m_MaxSpeed;
	InState.iRotateSpeed    = m_ParticleState.m_RotateSpeed;
	InState.isActive        = m_ParticleState.isActive;
	InState.isLooping       = m_ParticleState.isLooping;
	InState.iParticleNum    = m_ParticleState.m_ParticleNum;
	InState.iTime           = 1.0f / FPS;
	InState.iTargetPosition = m_TargetPos;
	InState.isChaser        = m_ParticleState.isChaser;
	InState.iMinChaseAngle  = m_ParticleState.m_MinChaseAngle;
	InState.iMaxChaseAngle  = m_ParticleState.m_MaxChaseAngle;
	InState.iGravity        = { m_ParticleState.m_Gravity[0],m_ParticleState.m_Gravity[1],m_ParticleState.m_Gravity[2] };
	InState.UseGravity      = m_ParticleState.UseGravity;

	if (m_CpSRV != nullptr) {
		m_CpSRV.Reset();
	}
	if (m_CpBuf != nullptr) {
		m_CpBuf.Reset();
	}
	ParticleSystemUtility::CreateStructuredBuffer(m_Device, sizeof(m_ParticleSRVState), 1, &InState, m_CpBuf.GetAddressOf());
	ParticleSystemUtility::CreateShaderResourceView(m_Device, m_CpBuf.Get(), m_CpSRV.GetAddressOf());
}

XMFLOAT4 ParticleSystem::RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d) {
	XMFLOAT3 Axis;//軸
	XMFLOAT4 q; //クォータニオン

	ParticleSystemMathUtil::DX11Vec3Cross(Axis, v0, v1);

	ParticleSystemMathUtil::DX11Vec3Dot(d, v0, v1);
	//ターゲットの方向と自機がほとんど一致したとき、内積の値が１を超える(-1を下回る)時があるので補正する
	if (d > 1.0) {
		d = 1.0;
	}
	if (d <= -1) {
		d = -1.0f;
	}

	float s = (float)sqrtf((1 + d) * 2);
	if (s == 0.0f) {
		ParticleSystemMathUtil::DX11QtIdentity(q);//ターゲットを追い越した
	}
	else {
		q.x = Axis.x / s;
		q.y = Axis.y / s;
		q.z = Axis.z / s;
		q.w = s / 2;
	}

	return q;
}

void ParticleSystem::StartNomalParticle() {
	isSystemActive = true;
	ParticlesDeathCount = 0;

	m_SystemLifeTime = m_ParticleState.m_DuaringTime + m_ParticleState.m_StartDelayTime;

	//変更されたステータス反映
	//{
	//	m_MaxParticleNum = m_ParticleState.m_ParticleNum;
	//}
	srand((int)NowTime);

	//パーティクル設定
	ParticleDetalInit();

}

void ParticleSystem::StartGPUParticle(){
	////GPUパーティクル設定---------------------------------

	m_ParticleNum = m_ParticleState.m_ParticleNum;
	if (m_CpUAV != nullptr) {
		m_CpUAV.Reset();
	}
	if (m_CpResult != nullptr) {
		m_CpResult.Reset();
	}

	//入力用バッファを更新
	UpdateSRV();
	//出力用バッファを更新
	ParticleSystemUtility::CreateStructuredBuffer(m_Device, sizeof(m_ParticleUAVState), m_ParticleNum, nullptr, m_CpResult.GetAddressOf());
	ParticleSystemUtility::CreateUnOrderAccessView(m_Device, m_CpResult.Get(), m_CpUAV.GetAddressOf());

	//初期化用コンピュートシェーダー実行
	ParticleSystemUtility::RunComputeShader(m_DeviceContext, m_InitComputeShader, 1, m_CpSRV.GetAddressOf(), m_CpUAV.Get(), m_ParticleNum, 1, 1);

	m_CpGetBuf = ParticleSystemUtility::CreateAndCopyToBuffer(m_Device, m_DeviceContext, m_CpResult.Get());

	m_DeviceContext->Map(m_CpGetBuf.Get(), 0, D3D11_MAP_READ, 0, &m_MappedSubResource);
	m_DeviceContext->Unmap(m_CpGetBuf.Get(), 0);
	////-------------------------------------------------

	m_SystemLifeTime = m_ParticleState.m_DuaringTime + m_ParticleState.m_StartDelayTime;
	isSystemActive = true;
	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
		m_ParticleVec.shrink_to_fit();
	}
}


//描画メソッド
void ParticleSystem::DrawNomal(const XMFLOAT4X4& CameraMatrix) {

	m_BillBoard.SetDrawUtility();
	for (auto iParticleDetail : m_ParticleDetails) {
		if (iParticleDetail.isAlive != true) {
			continue;
		}

		//描画
		{
			m_BillBoard.SetPosition(iParticleDetail.Matrix._41, iParticleDetail.Matrix._42, iParticleDetail.Matrix._43);
			m_BillBoard.SetSize(m_ParticleState.m_Size, m_ParticleState.m_Size);
			m_BillBoard.SetColor(XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]));
			m_BillBoard.DrawOnly(CameraMatrix, (float)iParticleDetail.ZAngle);
		}
	}
}

//GPUパーティクル用の描画メソッド
void ParticleSystem::GPUDraw(const XMFLOAT4X4& CameraMatrix) {

	m_BillBoard.SetDrawUtility();
	for (int Count = 0; Count < m_ParticleNum; Count++) {
		if (OutState[Count].isAlive == false || OutState[Count].isWaiting == 1) {
			continue;
		}

		//描画
		m_BillBoard.SetPosition(OutState[Count].Matrix._41, OutState[Count].Matrix._42, OutState[Count].Matrix._43);
		m_BillBoard.SetSize(m_ParticleState.m_Size, m_ParticleState.m_Size);
		m_BillBoard.SetColor(XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]));
		m_BillBoard.DrawOnly(CameraMatrix, (float)OutState[Count].ZAngle);
	}
}

void ParticleSystem::UnInit() {
	//終了処理
	if (Particles != nullptr) {
		delete[] Particles;
		Particles = nullptr;
	}

	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
		m_ParticleVec.shrink_to_fit();
	}

	if (m_ParticleDetails.empty() != true) {
		m_ParticleDetails.clear();
		m_ParticleDetails.shrink_to_fit();
	}
}

void ParticleSystem::AddParticle(m_Particles* AddParticle) {
	//パーティクルリストに追加
	m_Particles newParticle;
	XMFLOAT4 qt;//クォータニオン

	XMFLOAT4 axisX, axisY, axisZ;//ベクトル保存
	XMFLOAT4 qtx, qty, qtz;//クォータニオン
	XMFLOAT4 tempqt1, tempqt2, tempqt3;//一時保存クォータニオン

	DX11MatrixIdentity_subDefiner(newParticle.Matrix);

	//角度を取り出し-------------------------------
	axisX.x = newParticle.Matrix._11;
	axisX.y = newParticle.Matrix._12;
	axisX.z = newParticle.Matrix._13;

	axisY.x = newParticle.Matrix._21;
	axisY.y = newParticle.Matrix._22;
	axisY.z = newParticle.Matrix._23;

	axisZ.x = newParticle.Matrix._31;
	axisZ.y = newParticle.Matrix._32;
	axisZ.z = newParticle.Matrix._33;
	//-------------------------------------------
		
	//行列からクォータニオンを生成
	ParticleSystemMathUtil::DX11GetQtfromMatrix(newParticle.Matrix, qt);

	//ランダムな角度をそれぞれ設定
	//指定軸回転のクォータニオンを生成
	ParticleSystemMathUtil::DX11QtRotationAxis(qtx, axisX, (float)(rand() % m_ParticleState.m_AngleRange) + m_ParticleState.m_Angle[0]);
	ParticleSystemMathUtil::DX11QtRotationAxis(qty, axisY, (float)(rand() % m_ParticleState.m_AngleRange) + m_ParticleState.m_Angle[1]);
	ParticleSystemMathUtil::DX11QtRotationAxis(qtz, axisZ, (float)(rand() % m_ParticleState.m_AngleRange) + m_ParticleState.m_Angle[2]);

	//クォータニオンを正規化

	//クォータニオンを合成

	ParticleSystemMathUtil::DX11QtMul(tempqt1, qt, qtx);

	ParticleSystemMathUtil::DX11QtMul(tempqt2, qty, qtz);

	ParticleSystemMathUtil::DX11QtMul(tempqt3, tempqt1, tempqt2);

	//クォータニオンをノーマライズ
	ParticleSystemMathUtil::DX11QtNormalize(tempqt3, tempqt3);

	ParticleSystemMathUtil::DX11MtxFromQt(newParticle.Matrix, tempqt3);

	newParticle.Matrix._41 = -1 * m_ParticleState.m_Position[0];
	newParticle.Matrix._42 = m_ParticleState.m_Position[1];
	newParticle.Matrix._43 = m_ParticleState.m_Position[2];

	//発生までの待機時間設定
	newParticle.DelayTime = 0;
	//その他設定
	newParticle.LifeTime = m_ParticleState.m_MaxLifeTime;

	newParticle.ZAngle = rand() % 360;
	
	newParticle.CountTime = 0;
	newParticle.isWaiting = false;
	newParticle.isAlive = true;

	memcpy_s(AddParticle, sizeof(m_Particles), &newParticle, sizeof(m_Particles));

	//パーティクルを追加
	//m_ParticleVec.push_back(newParticle);

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
		
		SetParticleSystemState(&GetState);//情報セット

		fclose(Fp);
	}
	catch (int i) {//ファイル読み込み失敗したら
		return i;
	};

	return true;
}

bool ParticleSystem::FInTex(const char* FileName_) {
	//テクスチャ読み込み
	std::string Texname = ".\\InPutData/";
	Texname += FileName_;//ファイルの位置を指定

	Init(m_Device, m_DeviceContext,&m_ParticleState, Texname.c_str());//指定したファイルを利用して初期化
	(this->*fpStartFunc)();
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

void ParticleSystem::RemoveNextParticle(int SystemNumber) {

	//次に起動するパーティクルシステムの番号リストから削除
	for (auto iNumber = m_NextParticleNumberVector.begin(); iNumber != m_NextParticleNumberVector.end();) {
		if (*iNumber == SystemNumber) {
			iNumber = m_NextParticleNumberVector.erase(iNumber);
		}
		else {
			iNumber++;
		}
	}
}

//アクセサ
void ParticleSystem::SetParticleSystemState(t_ParticleSystemState* SetParticleSystemState_) {
	//パーティクルシステムの設定を反映させる
	if (SetParticleSystemState_ == nullptr) {
		return;
	}
	memcpy(&m_ParticleState, SetParticleSystemState_, sizeof(t_ParticleSystemState));
	ChangeGPUParticleMode(SetParticleSystemState_->isGPUParticle);
}

//名前設定
void ParticleSystem::SetName(const char* setName) {
	strcpy_s(m_ParticleState.m_Name, setName);
}

ParticleSystem& ParticleSystem::SetActive(bool set) {
	m_ParticleState.isActive = set;
	return *this;
}

ParticleSystem& ParticleSystem::SetEmitte(bool set) {
	isEmitting = set;
	return *this;
}

//パーティクル終了時に呼び出す別パーティクルシステム設定
void ParticleSystem::SetNextParticleSystem(int NextNumber) {
	//m_ParticleState.m_NextSystemNumber = NextNumber;
	if (NextNumber == -1) {
		m_NextParticleNumberVector.clear();
		m_NextParticleNumberVector.shrink_to_fit();
		return;
	}

	m_NextParticleNumberVector.emplace_back(NextNumber);
	//ソートして連続した重複要素を削除
	std::sort(m_NextParticleNumberVector.begin(), m_NextParticleNumberVector.end());
	m_NextParticleNumberVector.erase(std::unique(m_NextParticleNumberVector.begin(), m_NextParticleNumberVector.end()), m_NextParticleNumberVector.end());
}

//メソッドでの処理そのものの有効化無効化
void ParticleSystem::SetisUpdateActive(bool active) {
	isUpdateActive = active;
}
void ParticleSystem::SetisDrawActive(bool active) {
	isDrawActive = active;
}

//コンピュートシェーダー設定
ParticleSystem& ParticleSystem::SetComputeShader(ID3D11ComputeShader* setShader, eComputeShaderType type) {
	switch (type)
	{
	case eComputeShaderType::INIT:
		m_InitComputeShader = setShader;
		break;

	case eComputeShaderType::UPDATE:
		m_ComputeShader = setShader;
		break;

	default:
		break;
	}
	return *this;
}

//目標座標を設定
void ParticleSystem::SetTargetPos(float x, float y, float z) {
	m_TargetPos.x = x;
	m_TargetPos.y = y;
	m_TargetPos.z = z;
}

//識別番号を設定
ParticleSystem& ParticleSystem::setSystemNumber(int setNumber) {
	m_ParticleState.m_SystemNumber = setNumber;
	return *this;
}

//ソフトパーティクル用定数バッファを設定
void ParticleSystem::SetSoftPConstantBuffer(ConstantBufferSoftParticle* setState) {
	if (setState != nullptr) {
		memcpy_s(&m_ParticleState.m_CBSoftParticleState, sizeof(ConstantBufferSoftParticle), setState, sizeof(ConstantBufferSoftParticle));
	}
	
	m_DeviceContext->UpdateSubresource(m_CpCBufferSoftParticle.Get(), 0, nullptr, &m_ParticleState.m_CBSoftParticleState, 0, 0);

	m_DeviceContext->PSSetConstantBuffers(5, 1, m_CpCBufferSoftParticle.GetAddressOf());
}

//ビューポート用定数バッファを設定
ParticleSystem& ParticleSystem::SetViewPort(float* viewport) {
	m_ParticleState.m_CBSoftParticleState.iViewPort[0] = viewport[0];
	m_ParticleState.m_CBSoftParticleState.iViewPort[1] = viewport[1];

	m_DeviceContext->UpdateSubresource(m_CpCBufferSoftParticle.Get(), 0, nullptr, &m_ParticleState.m_CBSoftParticleState, 0, 0);

	m_DeviceContext->PSSetConstantBuffers(5, 1, m_CpCBufferSoftParticle.GetAddressOf());

	return *this;
}

//getter
float* ParticleSystem::getMatrixf16() {
	//TODO 回転移動の反映
	float retMat[16] = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		m_ParticleState.m_Position[0], m_ParticleState.m_Position[1], m_ParticleState.m_Position[2], 1.f
	};

	return retMat;
}

t_ParticleSystemState ParticleSystem::GetState() {
	return m_ParticleState;
}

char* ParticleSystem::getName() {
	return m_ParticleState.m_Name;
}

int ParticleSystem::getSystemNumber() {
	return m_ParticleState.m_SystemNumber;
}

int ParticleSystem::getNextSystemNumber() {
	return m_ParticleState.m_NextSystemNumber;
}

std::vector<int> ParticleSystem::getNextSystemNumbers() {
	return m_NextParticleNumberVector;
}

//メソッドでの処理そのものの有効化無効化
bool ParticleSystem::getisUpdateActive() const {
	return isUpdateActive;
}
bool ParticleSystem::getisDrawActive() const {
	return isDrawActive;
}


float ParticleSystem::getLifeTime() {
	return m_SystemLifeTime;
}

ConstantBufferSoftParticle ParticleSystem::getCBSoftParticleState() {
	return m_ParticleState.m_CBSoftParticleState;
}

void ParticleSystem::ChangeGPUParticleMode(bool isGPUMode) {
	//パーティクルの動作モードによって実行するメソッドを切り替える
	if (isGPUMode) {
		fpStartFunc = &ParticleSystem::StartGPUParticle;
		fpUpdateFunc = &ParticleSystem::UpdateComputeShader;
		fpDrawFunc = &ParticleSystem::GPUDraw;
	}
	else {
		fpStartFunc = &ParticleSystem::StartNomalParticle;
		fpUpdateFunc = &ParticleSystem::UpdateNomal;
		fpDrawFunc = &ParticleSystem::DrawNomal;
	}
}

void ParticleSystem::ChangeSoftParticleMode(bool isSoftParticle) {
	//ソフトパーティクルかどうかによって利用するシェーダーを切り替え、ビルボードを初期化する
	m_ParticleState.isSoftParticle = isSoftParticle;
	if (isSoftParticle) {
		m_BillBoard.Init(m_Device, m_DeviceContext, 0, 0, 0,
			m_ParticleState.m_Size, m_ParticleState.m_Size,
			XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]),
			SOFTPARTICLE_PS_SHADER,
			SOFTPARTICLE_VS_SHADER);
	}
	else {
		m_BillBoard.Init(m_Device, m_DeviceContext, 0, 0, 0,
			m_ParticleState.m_Size, m_ParticleState.m_Size,
			XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]),
			PARTICLE_PS_SHADER,
			PARTICLE_VS_SHADER);
	}
}