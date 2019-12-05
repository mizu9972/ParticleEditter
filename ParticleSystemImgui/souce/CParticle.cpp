//パーティクルシステムプログラム

//ヘッダー
#include <random>
#include "CParticle.h"
#include "dx11mathutil.h"
#include "CCamera.h"
#include "CDirectxGraphics.h"

//デバッグ用
#include "CTimer.h"


#define		SCREEN_X		1200
#define		SCREEN_Y		600
#define		PARTICLE_NUM_PER_THREAD (64) //csParticle.hlsl内と同じ定数を宣言
#define		THREAD_NUM (256)

#ifndef     ALIGN16
#define     ALIGN16 _declspec(align(16))
#endif

constexpr auto PARTICLE_TEXTURE = "assets/ParticleTexture/particle.png";
constexpr auto PARTICLE_PS_SHADER = "shader/psParticle.fx";
constexpr auto PARTICLE_VS_SHADER = "shader/vsPrticle.fx";
/*------------------------
単位行列にする
--------------------------*/
void DX11MatrixIdentity_subDefiner(DirectX::XMFLOAT4X4& mat) {

	ALIGN16 XMMATRIX mtx;

	mtx = XMMatrixIdentity();

	XMStoreFloat4x4(&mat, mtx);
}

//メソッド
//初期化
void ParticleSystem::Init() {

	//パーティクル初期化
	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
	}

	//初期ステータスを設定
	t_ParticleSystemState newState;
	SetParticleSystemState(&newState);
	
	InitComputeShader();
	
	//ビルボード初期化---------------------------------------------------------------------------------------------------------------
	m_BillBoard.Init(0, 0, 0,
		m_ParticleState.m_Size, m_ParticleState.m_Size,
		XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]),
		PARTICLE_PS_SHADER,
		PARTICLE_VS_SHADER);
	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };

	m_BillBoard.SetUV(u, v);
	m_BillBoard.LoadTexTure(m_ParticleState.m_TextureName);
	//----------------------------------------------------------------------------------------------------------------------------


	Start();
}

void ParticleSystem::Init(t_ParticleSystemState* ParticleState_) {
	//パーティクル初期化
	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
	}

	//引数のステータスを設定
	SetParticleSystemState(ParticleState_);

	InitComputeShader();

	//ビルボード初期化---------------------------------------------------------------------------------------------------------------
	m_BillBoard.Init(0, 0, 0,
		ParticleState_->m_Size, ParticleState_->m_Size,
		XMFLOAT4(ParticleState_->m_Color[0], ParticleState_->m_Color[1], ParticleState_->m_Color[2], ParticleState_->m_Color[3]),
		PARTICLE_PS_SHADER,
		PARTICLE_VS_SHADER);

	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
	m_BillBoard.SetUV(u, v);
	m_BillBoard.LoadTexTure(m_ParticleState.m_TextureName);
	//----------------------------------------------------------------------------------------------------------------------------
	Start();
}
void ParticleSystem::Init(t_ParticleSystemState ParticleState_, const char* filename, ID3D11Device* device) {
	//パーティクル初期化
	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
	}

	//引数のステータスを設定
	SetParticleSystemState(&ParticleState_);

	// ファイル名を保存
	strcpy_s(m_ParticleState.m_TextureName, filename);

	InitComputeShader();

	//ビルボード初期化---------------------------------------------------------------------------------------------------------------
	m_BillBoard.Init(0, 0, 0,
		m_ParticleState.m_Size, m_ParticleState.m_Size, 
		XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]),
		PARTICLE_PS_SHADER,
		PARTICLE_VS_SHADER);

	float u[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float v[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
	m_BillBoard.SetUV(u, v);
	m_BillBoard.LoadTexTure(m_ParticleState.m_TextureName);
	//----------------------------------------------------------------------------------------------------------------------------
	Start();
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
	ID3D11Device* device = CDirectXGraphics::GetInstance()->GetDXDevice();
	ID3D11DeviceContext* devicecontext = CDirectXGraphics::GetInstance()->GetImmediateContext();	

	m_CpSRV.Attach(m_pSRV);
	m_CpUAV.Attach(m_pUAV);
	m_CpResult.Attach(m_pResult);
	m_CpConstantBuffer.Attach(m_ConstantBuffer);
	m_CpGetBuf.Attach(getbuf);

	CreateConstantBuffer(CDirectXGraphics::GetInstance()->GetDXDevice(),
		sizeof(m_ConstantBufferParticle),
		&m_CpConstantBuffer);

}

void ParticleSystem::Update() {
	//更新処理
	(this->*fpUpdateFunc)();
}

void ParticleSystem::UpdateComputeShader() {

	if (m_ParticleNum <= 0) {
		return;
	}

	ID3D11DeviceContext* devicecontext = CDirectXGraphics::GetInstance()->GetImmediateContext();

	//コンピュートシェーダーを実行
	const UINT dispatchX = UINT(ceil(float(m_ParticleNum) / float(THREAD_NUM * PARTICLE_NUM_PER_THREAD)));
	RunComputeShader(devicecontext, m_ComputeShader, 1, m_CpSRV.GetAddressOf(), m_CpUAV.Get(),dispatchX, 1, 1);

	//データ受け取り
	devicecontext->CopyResource(m_CpGetBuf.Get() , m_CpResult.Get());//バッファコピー
	devicecontext->Map(m_CpGetBuf.Get(), 0, D3D11_MAP_READ, 0, &m_MappedSubResource);

	OutState = reinterpret_cast<m_ParticleUAVState*>(m_MappedSubResource.pData);//データ獲得

	devicecontext->Unmap(m_CpGetBuf.Get(), 0);
}

void ParticleSystem::UpdateNomal() {

	m_Particles* p_PickParticle;//操作するパーティクルのポインタ保存用

	//float lng;
	XMFLOAT4 TargetQt;//ターゲット方向の姿勢
	XMFLOAT3 TargetVector;

	float Speed_ = m_ParticleState.m_Speed;

	NowTime = 1.0f / FPS;

	//パーティクル更新
	for (int ParticleNum = 0; ParticleNum < m_ParticleVec.size(); ParticleNum++) {

		if (m_ParticleState.isChaser) {

			//座標保存
			XMFLOAT3 ParticlePosition = XMFLOAT3(m_ParticleVec[ParticleNum].Matrix._41, m_ParticleVec[ParticleNum].Matrix._42, m_ParticleVec[ParticleNum].Matrix._43);
			
			//現在位置からターゲット方向へのベクトルを求める
			TargetVector.x = m_TargetPos.x - m_ParticleVec[ParticleNum].Matrix._41;
			TargetVector.y = m_TargetPos.y - m_ParticleVec[ParticleNum].Matrix._42;
			TargetVector.z = m_TargetPos.z - m_ParticleVec[ParticleNum].Matrix._43;
			//DX11GetQtfromMatrix(m_ParticleVec[ParticleNum].Matrix, Quaternion);

			XMFLOAT3 ZDir = XMFLOAT3(m_ParticleVec[ParticleNum].Matrix._31, m_ParticleVec[ParticleNum].Matrix._32, m_ParticleVec[ParticleNum].Matrix._33);//ミサイルの方向ベクトル(Z方向)

			//正規化
			DX11Vec3Normalize(TargetVector, TargetVector);
			DX11Vec3Normalize(ZDir, ZDir);
			DX11GetQtfromMatrix(m_ParticleVec[ParticleNum].Matrix, m_Quaternion);

			float Dot;//２本のベクトルの内積値
			TargetQt = RotationArc(ZDir, TargetVector, Dot);//２本のベクトルから為す角度とクォータニオンを求める
			float AngleDiff = acosf(Dot);//ラジアン角度
			float AngleMax = XM_PI * m_ParticleState.m_MaxChaseAngle / 180.0f;
			float AngleMin = XM_PI * m_ParticleState.m_MinChaseAngle / 180.0f;
			//パーティクルの姿勢を決定する
			if (AngleMin > AngleDiff) {

			}else if (AngleMax >= AngleDiff) {
				//角度の差が更新できる角度より大きいか小さいか
				DX11QtMul(m_Quaternion, m_Quaternion, TargetQt);
			}
			else {
				float t = AngleMax / AngleDiff;

				XMFLOAT4 toqt;
				DX11QtMul(toqt, m_Quaternion, TargetQt);
				DX11QtSlerp(m_Quaternion, toqt, t, m_Quaternion);
			}

			//クォータニオンを行列にする
			//現在の姿勢をクォータニオンにする
			DX11MtxFromQt(m_ParticleVec[ParticleNum].Matrix, m_Quaternion);

			m_ParticleVec[ParticleNum].Matrix._41 = ParticlePosition.x;
			m_ParticleVec[ParticleNum].Matrix._42 = ParticlePosition.y;
			m_ParticleVec[ParticleNum].Matrix._43 = ParticlePosition.z;

		}

		m_ParticleVec[ParticleNum].CountTime += NowTime;

		//重力計算
		if (m_ParticleState.UseGravity) {
			m_ParticleVec[ParticleNum].Matrix._31 += m_ParticleState.m_Gravity[0] / 100.0f;
			m_ParticleVec[ParticleNum].Matrix._32 += m_ParticleState.m_Gravity[1] / 100.0f;
			m_ParticleVec[ParticleNum].Matrix._33 += m_ParticleState.m_Gravity[2] / 100.0f;
		}


		//速度分移動させる
		m_ParticleVec[ParticleNum].Matrix._41 += m_ParticleVec[ParticleNum].Matrix._31 * Speed_ * NowTime;
		m_ParticleVec[ParticleNum].Matrix._42 += m_ParticleVec[ParticleNum].Matrix._32 * Speed_ * NowTime;
		m_ParticleVec[ParticleNum].Matrix._43 += m_ParticleVec[ParticleNum].Matrix._33 * Speed_ * NowTime;


		//生存時間減少
		m_ParticleVec[ParticleNum].LifeTime -= NowTime;

		m_ParticleVec[ParticleNum].ZAngle += m_ParticleState.m_RotateSpeed;
		m_ParticleVec[ParticleNum].ZAngle = m_ParticleVec[ParticleNum].ZAngle % 360;

		//死亡判定
		if (m_ParticleVec[ParticleNum].LifeTime <= 0) {
			m_ParticleVec[ParticleNum].isAlive = false;
			m_ParticleVec.erase(m_ParticleVec.begin() + ParticleNum);//配列から削除
			m_ParticleVec.shrink_to_fit();//メモリ解放
			ParticlesDeathCount += 1;
		}
	}

	m_SystemLifeTime -= NowTime;

	//パーティクル終了判定
	if (m_SystemLifeTime <= 0) {
		if (isSystemActive != true) {
			return;
		}
		isSystemActive = false;

		if (m_ParticleState.m_NextSystemNumber != -1) {
			//次のパーティクル開始
			Notify(this);
		}

		if (m_ParticleState.isLooping == true) {//ループ
			Start();
		}

	}
	if (isSystemActive == false) {
		return;
	}


	//待機中のパーティクル
	for (int ParticlesNum = 0; ParticlesNum < m_MaxParticleNum; ParticlesNum++) {

		p_PickParticle = &Particles[ParticlesNum];

		//待機中なら待機時間減少
		if (p_PickParticle->isWaiting == true) {
			p_PickParticle->DelayTime -= NowTime;

			//待機時間が0以下なら活性化
			if (p_PickParticle->DelayTime <= 0) {
				p_PickParticle->isWaiting = false;
				p_PickParticle->isAlive = true;

				AddParticle(p_PickParticle);
			}
		}
	}
	
}

void ParticleSystem::UpdateConstantBuffer() {
	//コンスタントバッファ更新-------------------------------------------------------------------------------------------------------
	{
		m_CbParticle.iPosition = { m_ParticleState.m_Position[0], m_ParticleState.m_Position[1],m_ParticleState.m_Position[2],0 };
		m_CbParticle.iAngle = { m_ParticleState.m_Angle[0],    m_ParticleState.m_Angle[1],   m_ParticleState.m_Angle[2],   0 };
		m_CbParticle.iAngleRange = m_ParticleState.m_AngleRange;
		m_CbParticle.iDuaringTime = m_ParticleState.m_DuaringTime;
		m_CbParticle.iMaxLifeTime = m_ParticleState.m_MaxLifeTime;
		m_CbParticle.iSpeed = m_ParticleState.m_Speed;
		m_CbParticle.iRotateSpeed = m_ParticleState.m_RotateSpeed;
		m_CbParticle.isActive = m_ParticleState.isActive;
		m_CbParticle.isLooping = m_ParticleState.isLooping;
		m_CbParticle.iParticleNum = m_ParticleState.m_ParticleNum;
		m_CbParticle.iTime = 1.0f / FPS;
		m_CbParticle.iTargetPosition = m_TargetPos;
		m_CbParticle.isChaser = m_ParticleState.isChaser;
		m_CbParticle.iMinChaseAngle = m_ParticleState.m_MinChaseAngle;
		m_CbParticle.iMaxChaseAngle = m_ParticleState.m_MaxChaseAngle;
		m_CbParticle.iGravity = { m_ParticleState.m_Gravity[0],m_ParticleState.m_Gravity[1],m_ParticleState.m_Gravity[2] };
		m_CbParticle.UseGravity = m_ParticleState.UseGravity;
	}
	CDirectXGraphics::GetInstance()->GetImmediateContext()->UpdateSubresource(
		m_CpConstantBuffer.Get(),
		0,
		NULL,
		&m_CbParticle,
		0,
		0
	);
	CDirectXGraphics::GetInstance()->GetImmediateContext()->CSSetConstantBuffers(
		7,
		1,
		m_CpConstantBuffer.GetAddressOf()
	);

	//--------------------------------------------------------------------------------------------------------------------------
}

XMFLOAT4 ParticleSystem::RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d) {
	XMFLOAT3 Axis;//軸
	XMFLOAT4 q; //クォータニオン

	DX11Vec3Cross(Axis, v0, v1);

	DX11Vec3Dot(d, v0, v1);
	//ターゲットの方向と自機がほとんど一致したとき、内積の値が１を超える(-1を下回る)時があるので補正する
	if (d > 1.0) {
		d = 1.0;
	}
	if (d <= -1) {
		d = -1.0f;
	}

	float s = (float)sqrtf((1 + d) * 2);
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

//パーティクル処理開始
void ParticleSystem::Start() {
	
	//パーティクル生成
	if (m_ParticleState.isActive == false) {
		return;
	}
	if (Particles != NULL) {
		delete[] Particles;
	}
	Particles = new m_Particles[m_ParticleState.m_ParticleNum];
	isSystemActive = true;
	ParticlesDeathCount = 0;

	m_SystemLifeTime = m_ParticleState.m_DuaringTime;
	
	//変更されたステータス反映
	{
		m_MaxParticleNum = m_ParticleState.m_ParticleNum;
	}
	//パーティクル設定-------------------------------------------------------------------------
	srand((int)NowTime);
	
	m_Particles* p_PickParticle;//操作するパーティクルのポインタ保存用

	//パーティクルの初期設定する
	for (int ParticlesNum = 0; ParticlesNum < m_MaxParticleNum; ParticlesNum++) {

		p_PickParticle = &Particles[ParticlesNum];

		//発生までの待機時間設定
		//パーティクルの発生時間をパーティクル数で等分し、それぞれ割り当てる
		p_PickParticle->DelayTime = m_ParticleState.m_DuaringTime / m_MaxParticleNum * ParticlesNum;

		p_PickParticle->CountTime = 0;
		//待機中に
		p_PickParticle->isAlive = false;
		p_PickParticle->isWaiting = true;
	}
	//--------------------------------------------------------------------------------------


}

void ParticleSystem::StartGPUParticle(){
	////GPUパーティクル設定---------------------------------

	ID3D11Device* device = CDirectXGraphics::GetInstance()->GetDXDevice();
	ID3D11DeviceContext* devicecontext = CDirectXGraphics::GetInstance()->GetImmediateContext();
	m_ParticleNum = m_ParticleState.m_ParticleNum;
	if (m_CpUAV != nullptr) {
		m_CpUAV->Release();
		m_CpUAV = nullptr;
	}
	if (m_CpResult != nullptr) {
		m_CpResult->Release();
		m_CpResult = nullptr;
	}
	////入力用バッファを更新
	//CreateStructuredBuffer(device, sizeof(m_ParticleSRVState), m_ParticleState.m_ParticleNum, nullptr, &m_pBuf);
	//CreateShaderResourceView(device, m_pBuf, &m_pSRV);
	//出力用バッファを更新
	CreateStructuredBuffer(device, sizeof(m_ParticleUAVState), m_ParticleNum, nullptr, m_CpResult.GetAddressOf());
	CreateUnOrderAccessView(device, m_CpResult.Get(), m_CpUAV.GetAddressOf());

	m_CpGetBuf = CreateAndCopyToBuffer(device, devicecontext, m_CpResult.Get());
	D3D11_MAPPED_SUBRESOURCE MappedSubResource;
	devicecontext->Map(m_CpGetBuf.Get(), 0, D3D11_MAP_READ, 0, &MappedSubResource);

	devicecontext->Unmap(m_CpGetBuf.Get(), 0);
	////-------------------------------------------------

	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
		m_ParticleVec.shrink_to_fit();
	}
}


//描画処理
void ParticleSystem::Draw(ID3D11DeviceContext* device) {

	(this->*fpDrawFunc)(device);
}

//描画メソッド
void ParticleSystem::DrawNomal(ID3D11DeviceContext* device) {

	m_BillBoard.SetDrawUtility();
	for (int ParticleNum = 0; ParticleNum < m_ParticleVec.size(); ParticleNum++) {
		if (m_ParticleVec[ParticleNum].isAlive == false) {
			continue;
		}

		//描画
		m_BillBoard.SetPosition(m_ParticleVec[ParticleNum].Matrix._41, m_ParticleVec[ParticleNum].Matrix._42, m_ParticleVec[ParticleNum].Matrix._43);
		m_BillBoard.SetSize(m_ParticleState.m_Size, m_ParticleState.m_Size);
		m_BillBoard.SetColor(XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]));
		m_BillBoard.DrawOnly(CCamera::GetInstance()->GetCameraMatrix(), (float)m_ParticleVec[ParticleNum].ZAngle);
	}
}

//GPUパーティクル用の描画メソッド
void ParticleSystem::GPUDraw(ID3D11DeviceContext* device) {

	m_BillBoard.SetDrawUtility();
	for (int Count = 0; Count < m_ParticleNum; Count++) {
		if (OutState[Count].isAlive == false || OutState[Count].isWaiting == 1) {
			continue;
		}

		//描画
		m_BillBoard.SetPosition(OutState[Count].Matrix._41, OutState[Count].Matrix._42, OutState[Count].Matrix._43);
		m_BillBoard.SetSize(m_ParticleState.m_Size, m_ParticleState.m_Size);
		m_BillBoard.SetColor(XMFLOAT4(m_ParticleState.m_Color[0], m_ParticleState.m_Color[1], m_ParticleState.m_Color[2], m_ParticleState.m_Color[3]));
		m_BillBoard.DrawOnly(CCamera::GetInstance()->GetCameraMatrix(), (float)OutState[Count].ZAngle);
	}
}

void ParticleSystem::UnInit() {

	if (Particles != nullptr) {
		delete[] Particles;
		Particles = nullptr;
	}

	if (m_ParticleVec.empty() != true) {
		m_ParticleVec.clear();
		m_ParticleVec.shrink_to_fit();
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
	DX11GetQtfromMatrix(newParticle.Matrix, qt);

	//ランダムな角度をそれぞれ設定
	//指定軸回転のクォータニオンを生成
	DX11QtRotationAxis(qtx, axisX, (float)(rand() % m_ParticleState.m_AngleRange) + m_ParticleState.m_Angle[0]);
	DX11QtRotationAxis(qty, axisY, (float)(rand() % m_ParticleState.m_AngleRange) + m_ParticleState.m_Angle[1]);
	DX11QtRotationAxis(qtz, axisZ, (float)(rand() % m_ParticleState.m_AngleRange) + m_ParticleState.m_Angle[2]);

	//クォータニオンを正規化

	//クォータニオンを合成

	DX11QtMul(tempqt1, qt, qtx);

	DX11QtMul(tempqt2, qty, qtz);

	DX11QtMul(tempqt3, tempqt1, tempqt2);

	//クォータニオンをノーマライズ
	DX11QtNormalize(tempqt3, tempqt3);

	DX11MtxFromQt(newParticle.Matrix, tempqt3);


	newParticle.Matrix._41 = m_ParticleState.m_Position[0];
	newParticle.Matrix._42 = m_ParticleState.m_Position[1];
	newParticle.Matrix._43 = m_ParticleState.m_Position[2];



	//発生までの待機時間設定
	//パーティクルの発生時間をパーティクル数で等分し、それぞれ割り当てる
	newParticle.DelayTime = 0;
	//その他設定
	newParticle.LifeTime = m_ParticleState.m_MaxLifeTime;

	newParticle.ZAngle = rand() % 360;
	
	//パーティクルを追加
	m_ParticleVec.push_back(newParticle);

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
/*
	Init(m_ParticleState, "assets/ParticleTexture/particle.png");*/
	return true;
}

bool ParticleSystem::FInTex(const char* FileName_) {
	//テクスチャ読み込み
	std::string Texname = ".\\InPutData/";
	Texname += FileName_;//ファイルの位置を指定

	Init(m_ParticleState, Texname.c_str(),CDirectXGraphics::GetInstance()->GetDXDevice());//指定したファイルを利用して初期化

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
void ParticleSystem::SetParticleSystemState(t_ParticleSystemState* SetParticleSystemState_) {
	memcpy(&m_ParticleState, SetParticleSystemState_, sizeof(t_ParticleSystemState));
}
void ParticleSystem::SetName(const char* setName) {
	strcpy_s(m_ParticleState.m_Name, setName);
}

ParticleSystem& ParticleSystem::SetActive(bool set) {
	m_ParticleState.isActive = set;
	return *this;
}

void ParticleSystem::SetNextParticleSystem(ParticleSystem* next) {
	//m_ParticleState.m_NextParticleSystem = next;
}

void ParticleSystem::SetNextParticleSystem(int NextNumber) {
	m_ParticleState.m_NextSystemNumber = NextNumber;
}

ParticleSystem& ParticleSystem::SetComputeShader(ID3D11ComputeShader* setShader) {
	m_ComputeShader = setShader;
	return *this;
}

void ParticleSystem::SetTargetPos(float x, float y, float z) {
	m_TargetPos.x = x;
	m_TargetPos.y = y;
	m_TargetPos.z = z;
}

ParticleSystem& ParticleSystem::setSystemNumber(int setNumber) {
	m_ParticleState.m_SystemNumber = setNumber;
	return *this;
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

void ParticleSystem::ChangeGPUParticleMode(bool isGPUMode) {
	//パーティクルの動作モードによって実行するメソッドを切り替える
	if (isGPUMode) {
		fpUpdateFunc = &ParticleSystem::UpdateComputeShader;
		fpDrawFunc = &ParticleSystem::GPUDraw;
	}
	else {
		fpUpdateFunc = &ParticleSystem::UpdateNomal;
		fpDrawFunc = &ParticleSystem::DrawNomal;
	}
}