#pragma once
#include	<directxmath.h>
#include	<string.h>
#include    <vector>

#include "CBillBoard.h"
#include "Observer.h"
#include <wrl.h>

#define     FPS 60

using namespace DirectX;
template<typename ComPtrT>
using ComPtr = Microsoft::WRL::ComPtr<ComPtrT>;

//ソフトパーティクル用コンスタントバッファ構造体
struct ConstantBufferSoftParticle {
	UINT iViewPort[2] = { 1600,900 };
	float iZfar = 0.1f;
	float iZVolume = 110.0f;
};

//パーティクルシステム設定構造体
//手動で設定して割り当てる
//一部の数値は動的に変更しても即時反映されず、パーティクル生成が一周終わってから反映される
typedef struct {
	char m_Name[64]         = "";//名前
	char m_TextureName[512] = "assets/ParticleTexture/particle.png";//テクスチャの名前
	
	float m_Position[3]     = { 0,0,0 };//座標
	int m_Angle[3]          = { 0,0,0 };//角度
	int m_AngleRange        = 360;//発生させる角度の範囲

	float m_StartDelayTime  = 0;//開始遅延時間
	float m_DuaringTime     = 10;//発生時間
	float m_Size            = 20;//大きさ
	float m_MaxLifeTime     = 10;//最長生存時間
	float m_Speed           = 50;//速度
	float m_Accel           = 0;//加速度
	float m_MinSpeed        = 0;//最小速度
	float m_MaxSpeed        = 100;//最大速度
	int m_ParticleNum       = 256;//生成するパーティクル個数 //即時反映されない

	float m_Color[4]        = {1.0f,1.0f,1.0f,1.0f};//パーティクルの色
	int m_RotateSpeed       = 1;//回転速度
	bool isChaser           = false;//ターゲットへ向かっていくパーティクルモード
	bool isActive           = true;//Startメソッドと同時に起動するかどうか(他のパーティクルシステムの後から発生させる場合はfalse)
	bool isEmitting         = false;//他パーティクルから発生させられるかどうか
	bool isLooping          = true;//ループするかどうか
	bool isGPUParticle      = false;//GPUパーティクルONOFF
	bool UseGravity         = false;//重力有効
	bool isSoftParticle		= false;//ソフトパーティクルにするかどうか
	ConstantBufferSoftParticle m_CBSoftParticleState;//コンスタントバッファに利用する構造体

	float m_Gravity[3]      = { 0,0,0 };//重力

	//ホーミング角度制限
	int m_MinChaseAngle     = 0;
	int m_MaxChaseAngle     = 5;

	int m_SystemNumber;//自身の番号(mapで管理するkeyになる)
	int m_NextSystemNumber  = -1;//次に発生させるパーティクルの番号
}t_ParticleSystemState;

class ParticleSystem:public Subject {
private:
	//生成するパーティクルの情報
	//自動で設定される
	struct m_Particles {
		float DelayTime;  //発射待機時間
		float LifeTime;   //生存時間
		XMFLOAT4X4 Matrix;//行列
		bool isAlive;     //生存しているか
		bool isWaiting;   //発生待機中か
		int ZAngle;       //回転
		float CountTime;  //経過時間
		int RandNum;      //ランダムに設定される数値
	};

	//コンピュートシェーダーに送る値
	struct m_ParticleSRVState {
		XMFLOAT4 iPosition;      //全体の位置
		XMINT4 iAngle;           //角度
		int iAngleRange;         //発射範囲
		float iDuaringTime;      //継続時間
		float iDelayTime;        //遅延時間
		float iMaxLifeTime;      //最大生存時間
		float iSpeed;            //速度
		float iAccel;            //加速度
		float iMinSpeed;         //最小速度
		float iMaxSpeed;         //最大速度
		int iRotateSpeed;        //回転速度
		int isActive;            //有効かどうか
		int isLooping;           //ループするかどうか
		int iParticleNum;        //パーティクルの個数
		float iTime;             //経過時間
		XMFLOAT3 iTargetPosition;//追いかけるターゲットの座標
		int isChaser;            //ターゲットを追いかけるか
		int iMinChaseAngle;      //追いかける最小角度
		int iMaxChaseAngle;      //追いかける最大角度

		int UseGravity;          //重力を利用するか
		XMFLOAT3 iGravity;       //重力
	};
	m_ParticleSRVState InState;

	//コンピュートシェーダー内で計算され、返ってくる値
	struct m_ParticleUAVState {
		XMFLOAT4X4 Matrix;
		float DelayTime   = 0;      //発射待機時間
		float LifeTime    = 0;      //生存時間
		float CountTime   = 0;      //経過時間
		int isAlive       = 1;      //生存しているか
		int isWaiting     = 0;      //発生待機中か
		int isInitialized = 0;		//初期化済みか
		int ZAngle        = 0;      //回転角度
		int RandNum       = 0;      //ランダムに設定される数値
	};
	m_ParticleUAVState* OutState;

	float NowTime;
	CBillBoard m_BillBoard;


	XMFLOAT3 m_TargetPos;
	XMFLOAT4 m_Quaternion; //クォータニオン
protected:
	t_ParticleSystemState* m_ParticleStateZero = nullptr;//初期値保存用
	t_ParticleSystemState m_ParticleState;//パーティクルシステム設定
	m_Particles* Particles = nullptr;

	std::vector<m_Particles> m_ParticleVec;//生存パーティクル配列
	std::vector<int> m_NextParticleNumberVector;

	int m_ParticleNum;//パーティクル個数保存用
	int m_MaxParticleNum;//パーティクル最大生成個数
	int ParticlesDeathCount;//死亡パーティクルカウント
	float m_SystemLifeTime;//パーティクルシステム残り時間
	bool isEmitting = true;
	bool isSystemActive = false;
	bool isUpdateActive = true;
	bool isDrawActive = true;
	
	//コンピュートシェーダー関連
	ID3D11Device* m_Device                        = nullptr;
	ID3D11DeviceContext* m_DeviceContext          = nullptr;
	ID3D11ComputeShader* m_ComputeShader          = nullptr;//コンピュートシェーダー(ParticleSystemParentで初期化されたものを受け取る)
	ID3D11ComputeShader* m_InitComputeShader      = nullptr;//初期化用コンピュートシェーダー(ParticleSystemParentで初期化されたものを受け取る)
	ID3D11Buffer* m_pResult                       = nullptr;//出力バッファ
	ID3D11Buffer* m_ConstantBufferSoftParticle    = nullptr;//ソフトパーティクル用コンスタントバッファ
	ID3D11Buffer* getbuf                          = nullptr;//バッファコピー用
	ID3D11Buffer* m_pbuf                          = nullptr;//シェーダーリソースビュー用バッファ
	ID3D11ShaderResourceView* m_pSRV              = nullptr;//シェーダーリソースビュー
	ID3D11UnorderedAccessView* m_pUAV             = nullptr;//アンオーダードアクセスビュー
	D3D11_MAPPED_SUBRESOURCE m_MappedSubResource;//コンピュートシェーダーから返ってくる数値

	//comポインタ宣言
	ComPtr<ID3D11Buffer> m_CpResult               = nullptr;//出力バッファ
	ComPtr<ID3D11Buffer> m_CpCBufferSoftParticle  = nullptr;//コンスタントバッファ
	ComPtr<ID3D11Buffer> m_CpGetBuf               = nullptr;//バッファコピー用
	ComPtr<ID3D11Buffer> m_CpBuf                  = nullptr;//シェーダーリソースビュー用バッファ
	ComPtr<ID3D11ShaderResourceView> m_CpSRV      = nullptr;//シェーダーリソースビュー
	ComPtr<ID3D11UnorderedAccessView> m_CpUAV     = nullptr;//アンオーダードアクセスビュー
public:
	ParticleSystem() {};
	ParticleSystem(t_ParticleSystemState ParticleState_) {
		memcpy(m_ParticleStateZero, &ParticleState_, sizeof(t_ParticleSystemState));
	};
	~ParticleSystem() {
		UnInit();
	};
	
	//コンピュートシェーダーの種類リスト
	enum class eComputeShaderType{
		INIT,
		UPDATE,
	};

	//基本処理メソッド

	//初期化
	ParticleSystem& Init(ID3D11Device* device, ID3D11DeviceContext* devicecontext,t_ParticleSystemState* ParticleState_ = nullptr, const char* filename = nullptr);
	void ZeroInit();//コンストラクタで数値設定した場合、生成時の状態に初期化できる
	void InitComputeShader();//コンピュートシェーダーの初期化

	//更新処理
	void Update();
	void (ParticleSystem::*fpUpdateFunc)() = &ParticleSystem::UpdateNomal;//関数ポインタ
	void UpdateNomal();
	void UpdateComputeShader();
	void UpdateSRV();

	XMFLOAT4 RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d);
	//パーティクル発生開始
	void Start();//パーティクル発生開始
	void (ParticleSystem::*fpStartFunc)() = &ParticleSystem::StartNomalParticle;
	void StartNomalParticle();
	void StartGPUParticle();
	
	//描画
	void Draw(const XMFLOAT4X4& CameraMatrix);
	void (ParticleSystem::*fpDrawFunc)(const XMFLOAT4X4& CameraMatrix) = &ParticleSystem::DrawNomal;//関数ポインタ
	void DrawNomal(const XMFLOAT4X4& CameraMatrix);
	void GPUDraw(const XMFLOAT4X4& CameraMatrix);

	//終了処理
	void UnInit();

	//パーティクル操作
	void AddParticle(m_Particles* AddParticle);
	void ChangeGPUParticleMode(bool isGPUMode = true);//GPUパーティクルモード変更
	void ChangeSoftParticleMode(bool isSoftParticle = true);//ソフトパーティクルモード切替

	//ファイル入出力メソッド
	bool FInState(const char* FileName_);
	void FOutState();
	bool FInTex(const char* FileName_);

	//アクセスメソッド
	//setter
	void SetName(const char*  setName);//名前
	ParticleSystem& SetActive(bool set);//有効無効
	ParticleSystem& SetEmitte(bool set);
	void SetTargetPos(float x, float y, float z);//ターゲット座標
	void SetParticleSystemState(t_ParticleSystemState* SetState_);//構造体情報全体
	void SetNextParticleSystem(int NextNumber);//次のパーティクルシステム番号
	void SetSoftPConstantBuffer(ConstantBufferSoftParticle* setState = nullptr);
	void SetisUpdateActive(bool active);
	void SetisDrawActive(bool active);
	ParticleSystem& SetViewPort(UINT* viewport);
	ParticleSystem& SetComputeShader(ID3D11ComputeShader* setShader, eComputeShaderType type);//コンピュートシェーダー
	ParticleSystem& setSystemNumber(int setNumber);//自身のパーティクルシステム番号
	//getter
	float* getMatrixf16();
	t_ParticleSystemState GetState();//構造体情報全体
	char* getName();//名前
	int getSystemNumber();//自身のパーティクルシステム番号
	int getNextSystemNumber();//次のパーティクルシステム番号
	std::vector<int> getNextSystemNumbers();
	bool getisUpdateActive()const;
	bool getisDrawActive()const;
	float getLifeTime();
	ConstantBufferSoftParticle getCBSoftParticleState();
};