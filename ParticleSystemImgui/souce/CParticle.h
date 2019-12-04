#pragma once
#include	<directxmath.h>
#include	<string.h>
#include    <vector>

#include "CBillBoard.h"
#include "Shader.h"
#include "Observer.h"
#include <wrl.h>

#define     FPS 60

using namespace DirectX;
class ParticleSystem;

//パーティクルシステム設定構造体
//手動で設定して割り当てる
//一部の数値は動的に変更しても即時反映されず、パーティクル生成が一周終わってから反映される
typedef struct {
	char m_Name[64]         = "";//名前
	char m_TextureName[512] = "assets/ParticleTexture/particle.png";//テクスチャの名前

	//座標
	float m_PositionX       = 0;
	float m_PositionY       = 0;
	float m_PositionZ       = 0;

	//角度
	int m_AngleX            = 0;
	int m_AngleY            = 0;
	int m_AngleZ            = 0;

	int m_AngleRange        = 360;//発生させる角度の範囲

	float m_DuaringTime     = 10;//発生時間
	float m_Size            = 20;//大きさ
	float m_MaxLifeTime     = 1;//最長生存時間
	float m_Speed           = 30;//速度
	int m_ParticleNum       = 256;//生成するパーティクル個数 //即時反映されない

	float m_Color[4]        = {1.0f,1.0f,1.0f,1.0f};//パーティクルの色
	int m_RotateSpeed       = 1;//回転速度
	bool isChaser           = false;//ターゲットへ向かっていくパーティクルモード
	bool isActive           = true;//Startメソッドと同時に起動するかどうか(他のパーティクルシステムの後から発生させる場合はfalse)
	bool isLooping          = true;//ループするかどうか
	bool isGPUParticle      = false;//GPUパーティクルONOFF

	//ParticleSystem* m_NextParticleSystem = nullptr;
	int m_SystemNumber;//自身の番号(mapで管理するkeyになる)
	int m_NextSystemNumber = -1;
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
		int isInitialied = 0;
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
	//std::vector<m_ParticleUAVState*> m_ParticleUAVvec;
	m_ParticleUAVState* OutState;

	//パーティクル全体共通のパラメータ
	//コンスタントバッファに流す
	struct m_ConstantBufferParticle {
		XMFLOAT4 iPosition;//全体の位置
		XMINT4 iAngle;//角度
		int iAngleRange;//発射範囲
		float iDuaringTime;//継続時間
		float iMaxLifeTime;//最大生存時間
		float iSpeed;//速度
		int iRotateSpeed;//回転速度
		int isActive;//有効かどうか
		int isLooping;//ループするかどうか
		int iParticleNum;//パーティクルの個数
		float iTime;//経過時間
		XMFLOAT3 iTargetPosition;//追いかけるターゲットの座標
		int isChaser;

		//バイト数調整用
		float Padding = 0;
		float Padding1 = 0;
		float Padding2 = 0;
	};
	m_ConstantBufferParticle m_CbParticle;

	float NowTime;
	CBillBoard m_BillBoard;


	XMFLOAT3 m_TargetPos;
	XMFLOAT4 m_Quaternion; //クォータニオン
protected:
	t_ParticleSystemState* m_ParticleStateZero = nullptr;//初期値保存用
	t_ParticleSystemState m_ParticleState;//パーティクルシステム設定
	m_Particles* Particles = nullptr;

	std::vector<m_Particles> m_ParticleVec;//生存パーティクル配列

	int m_ParticleNum;//パーティクル個数保存用
	int m_MaxParticleNum;//パーティクル最大生成個数
	int ParticlesDeathCount;//死亡パーティクルカウント
	float m_SystemLifeTime;//パーティクルシステム残り時間
	bool isSystemActive = false;

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	//コンピュートシェーダー関連
	ID3D11ComputeShader* m_ComputeShader          = nullptr;//コンピュートシェーダー
	//ComPtr<ID3D11ComputeShader> m_ComputeShader = nullptr;//コンピュートシェーダー
	ID3D11ComputeShader* m_InitComputeShader      = nullptr;//初期化コンピュートシェーダー
	ID3D11Buffer* m_pBuf                          = nullptr;//入力バッファ
	ID3D11Buffer* m_pResult                       = nullptr;//出力バッファ
	ID3D11Buffer* m_ConstantBuffer                = nullptr;//コンスタントバッファ
	ID3D11Buffer* getbuf = nullptr;//バッファコピー用
	D3D11_MAPPED_SUBRESOURCE m_MappedSubResource;//コンピュートシェーダーから返ってくる数値
	ID3D11ShaderResourceView* m_pSRV              = nullptr;//シェーダーリソースビュー
	ID3D11UnorderedAccessView* m_pUAV             = nullptr;//アンオーダードアクセスビュー
public:
	ParticleSystem() {};
	ParticleSystem(t_ParticleSystemState ParticleState_) {
		memcpy(m_ParticleStateZero, &ParticleState_, sizeof(t_ParticleSystemState));
	};
	~ParticleSystem() {
		UnInit();
	};
	
	//基本処理メソッド

	//初期化
	void Init();
	void Init(t_ParticleSystemState* ParticleState_);
	void Init(t_ParticleSystemState ParticleState_, const char* filename, ID3D11Device* device);
	void ZeroInit();//コンストラクタで数値設定した場合、生成時の状態に初期化できる
	void InitComputeShader();//コンピュートシェーダーの初期化

	//更新処理
	void Update();
	void (ParticleSystem::*fpUpdateFunc)() = &ParticleSystem::UpdateNomal;//関数ポインタ
	void UpdateNomal();
	void UpdateComputeShader();

	XMFLOAT4 RotationArc(XMFLOAT3 v0, XMFLOAT3 v1, float& d);
	//パーティクル発生開始
	void Start();//パーティクル発生開始
	void StartGPUParticle();
	
	//描画
	void Draw(ID3D11DeviceContext* device);
	void (ParticleSystem::*fpDrawFunc)(ID3D11DeviceContext* device) = &ParticleSystem::DrawNomal;//関数ポインタ
	void DrawNomal(ID3D11DeviceContext* device);
	void GPUDraw(ID3D11DeviceContext* device);

	//終了処理
	void UnInit();

	//パーティクル操作
	void AddParticle(m_Particles* AddParticle);
	void AddGPUParticle(m_Particles* AddParticle);
	void ChangeGPUParticleMode(bool isGPUMode = true);//GPUパーティクルモード変更

	//ファイル入出力メソッド
	bool FInState(const char* FileName_);
	void FOutState();
	bool FInTex(const char* FileName_);

	//アクセスメソッド
	//setter
	void SetSize(float Size_);
	void SetLifeTime(float LifeTime_);
	void SetSpeed(float Speed_);
	void SetName(const char*  setName);
	void SetFileName(const char* FileName_);
	ParticleSystem& SetActive(bool set);
	void SetTargetPos(float x, float y, float z);
	void SetParticleSystemState(t_ParticleSystemState* SetState_);
	void SetNextParticleSystem(ParticleSystem* next);
	void SetNextParticleSystem(int NextNumber);
	ParticleSystem& SetComputeShader(ID3D11ComputeShader* setShader);
	ParticleSystem& setSystemNumber(int setNumber);
	//getter
	t_ParticleSystemState GetState();
	char* getName();
	int getSystemNumber();
	int getNextSystemNumber();
};