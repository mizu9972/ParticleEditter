#include "Quo.fx"

#ifndef PARTICLE_COMMON
#define PARTICLE_COMMON



//cbuffer ConstantBufferParticle : register(b7)
//{
//    float4 iPosition;
//    int4 iAngle;
//    int iAngleRange;
//    float iDuaringTime;
//    float iMaxLifeTime;
//    float iSpeed;
//    int iRotateSpeed;
//    int isActive;
//    int isLooping;

//    int iParticleNum;
//    float iTime;
//    float3 iTargetPos;

//    int isChaser;
//    int iMinChaseAngle;
//    int iMaxChaseAngle;

//    int UseGravity;
//    float3 iGravity;
//    //バイト数調整用
//    //float Padding = 0;
//    //float Padding1 = 0;
//    //float Padding2 = 0;
//}

struct ParticleSRVState
{
    float4 iPosition; //全体の位置
    int4 iAngle; //角度
    int iAngleRange; //発射範囲
    float iDuaringTime; //継続時間
    float iMaxLifeTime; //最大生存時間
    float iSpeed; //速度
    float iAccel;
    int iRotateSpeed; //回転速度
    int isActive; //有効かどうか
    int isLooping; //ループするかどうか
    int iParticleNum; //パーティクルの個数
    float iTime; //経過時間
    float3 iTargetPosition; //追いかけるターゲットの座標
    int isChaser;
    int iMinChaseAngle;
    int iMaxChaseAngle;

    int UseGravity;
    float3 iGravity;
};

StructuredBuffer<ParticleSRVState> g_InState : register(t0);

struct ParticleUAVState
{

    Matrix m_Matrix; //行列
    float DelayTime; //発射待機時間
    float LifeTime; //生存時間
    float CountTime; //経過時間
    int isAlive; //生存しているか
    int isWaiting; //発生待機中か
    int isInitialized;
    int ZAngle; //回転
    int RandNum; //ランダムに設定される数値
};

RWStructuredBuffer<ParticleUAVState> g_OutState : register(u0);

//ランダムな値を返す
int fRandom(float x)
{
    return frac(sin(x) * 1000) * 1000;
}

//パーティクルの初期化
void ParticleInit(uint DTid_)
{
    float4 setQuot;
    float4 QtX, QtY, QtZ; //指定軸回転のクォータニオン

    //ランダムな角度の回転軸を生成
    QtX = CreateRotateQt(radians(fmod(fRandom(DTid_ + g_InState[0].iTime), g_InState[0].iAngleRange) + g_InState[0].iAngle.x), 1, 0, 0);
    QtY = CreateRotateQt(radians(fmod(fRandom(DTid_ + g_InState[0].iTime + 1), g_InState[0].iAngleRange) + g_InState[0].iAngle.y), 0, 1, 0);
    QtZ = CreateRotateQt(radians(fmod(fRandom(DTid_ + g_InState[0].iTime + 2), g_InState[0].iAngleRange) + g_InState[0].iAngle.z), 0, 0, 1);
    
    
    setQuot = MulQuot(MulQuot(QtX, QtY), QtZ); //合成

    g_OutState[DTid_].m_Matrix = QttoMatrix(setQuot); //行列に変換して反映

    g_OutState[DTid_].m_Matrix._14 = g_InState[0].iPosition.x;
    g_OutState[DTid_].m_Matrix._24 = g_InState[0].iPosition.y;
    g_OutState[DTid_].m_Matrix._34 = g_InState[0].iPosition.z;
   

    g_OutState[DTid_].LifeTime = g_InState[0].iMaxLifeTime;

    
    g_OutState[DTid_].ZAngle = fRandom(DTid_) % 360;

    g_OutState[DTid_].DelayTime = g_InState[0].iDuaringTime / g_InState[0].iParticleNum * DTid_;
    g_OutState[DTid_].CountTime = 0;
    g_OutState[DTid_].isAlive = true;
    g_OutState[DTid_].isWaiting = true;
    g_OutState[DTid_].RandNum = fRandom(DTid_);
    g_OutState[DTid_].isInitialized = 1;

}

//ターゲットを追いかけるパーティクル動作処理
//移動の向きをクォータニオンで変更させる
void TargetChase(uint DTid_)
{
    //行列全体が変更されるので座標を保存する
    float3 Position = float3(g_OutState[DTid_].m_Matrix._14, g_OutState[DTid_].m_Matrix._24, g_OutState[DTid_].m_Matrix._34);
    float3 ZDir = float3(g_OutState[DTid_].m_Matrix._13, g_OutState[DTid_].m_Matrix._23, g_OutState[DTid_].m_Matrix._33);
    float3 TargetPos = /* -1.0f * */g_InState[0].iTargetPosition.xyz;
    float3 TargetVector = -1.0f * (TargetPos.xyz - Position.xyz); //float3(TargetPos.x - Position.x, TargetPos.y - Position.y, TargetPos.z - Position.z); //距離計算

    matrix Mat = transpose(g_OutState[DTid_].m_Matrix);//行列を転置

    ZDir = normalize(ZDir);
    TargetVector = normalize(TargetVector);
    
    float4 Quoternion = CreateQtfromMatrix(g_OutState[DTid_].m_Matrix); //行列からクォータニオン作成
    float4 TransQuoternion = CreateQtfromMatrix(Mat);
    float4 TargetQuoternion;
    float Dot;

    Dot = dot(ZDir, TargetVector);
    
    Dot = min(Dot, 1.0f);
    Dot = max(Dot, -1.0f);

    TargetQuoternion = RotationArc(ZDir, TargetVector, Dot);
    float AngleDiff = acos(Dot);//ラジアン角度
    float AngleMax = (3.141592741f) * g_InState[0].iMaxChaseAngle / 180.0f;
    float AngleMin = (3.141592741f) * g_InState[0].iMinChaseAngle / 180.0f;
    //姿勢を決定
    if (AngleMin > AngleDiff)
    {

    }
    else if (AngleMax >= AngleDiff)
    {
        Quoternion = MultiplyQuot(Quoternion, TargetQuoternion);

    }
    else
    {
        float t = AngleMax / AngleDiff;

        float4 toqt;
        toqt = MultiplyQuot(Quoternion, TargetQuoternion);
        
        Quoternion = SlerpQt(Quoternion, toqt, t);

   }

    //クォータニオンを行列にする
    g_OutState[DTid_].m_Matrix = QttoMatrix(Quoternion);

    //保存していた座標を反映
    g_OutState[DTid_].m_Matrix._14 = Position.x;
    g_OutState[DTid_].m_Matrix._24 = Position.y;
    g_OutState[DTid_].m_Matrix._34 = Position.z;

}

//パーティクルの更新
void ParticleUpdate(uint DTid_)
{
    float Speed;
    if (g_OutState[DTid_].isWaiting == true)
    {
        g_OutState[DTid_].DelayTime -= g_InState[0].iTime;
        
        if (g_OutState[DTid_].DelayTime <= 0)
        {
            g_OutState[DTid_].isWaiting = false;
        }

        return;
    }

    if (g_InState[0].isChaser == 1)
    {
        TargetChase(DTid_);
    }

    if (g_InState[0].UseGravity == 1)
    {
        g_OutState[DTid_].m_Matrix._13 += g_InState[0].iGravity.x / 100.0f;
        g_OutState[DTid_].m_Matrix._23 += g_InState[0].iGravity.y / 100.0f;
        g_OutState[DTid_].m_Matrix._33 += g_InState[0].iGravity.z / 100.0f;

    }
    g_OutState[DTid_].CountTime += 1.0f / 60.0f;

    Speed = g_InState[0].iSpeed + g_InState[0].iAccel * g_OutState[DTid_].CountTime * g_OutState[DTid_].CountTime;
    //移動計算
    g_OutState[DTid_].m_Matrix._14 += g_OutState[DTid_].m_Matrix._13 * Speed * g_InState[0].iTime;
    g_OutState[DTid_].m_Matrix._24 += g_OutState[DTid_].m_Matrix._23 * Speed * g_InState[0].iTime;
    g_OutState[DTid_].m_Matrix._34 += g_OutState[DTid_].m_Matrix._33 * Speed * g_InState[0].iTime;

    //生存時間
    g_OutState[DTid_].LifeTime -= g_InState[0].iTime;

    if (g_OutState[DTid_].LifeTime <= 0)
    {
        g_OutState[DTid_].isAlive = false;
    }

}

#endif