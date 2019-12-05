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
//    //�o�C�g�������p
//    //float Padding = 0;
//    //float Padding1 = 0;
//    //float Padding2 = 0;
//}

struct ParticleSRVState
{
    float4 iPosition; //�S�̂̈ʒu
    int4 iAngle; //�p�x
    int iAngleRange; //���˔͈�
    float iDuaringTime; //�p������
    float iMaxLifeTime; //�ő吶������
    float iSpeed; //���x
    float iAccel;
    int iRotateSpeed; //��]���x
    int isActive; //�L�����ǂ���
    int isLooping; //���[�v���邩�ǂ���
    int iParticleNum; //�p�[�e�B�N���̌�
    float iTime; //�o�ߎ���
    float3 iTargetPosition; //�ǂ�������^�[�Q�b�g�̍��W
    int isChaser;
    int iMinChaseAngle;
    int iMaxChaseAngle;

    int UseGravity;
    float3 iGravity;
};

StructuredBuffer<ParticleSRVState> g_InState : register(t0);

struct ParticleUAVState
{

    Matrix m_Matrix; //�s��
    float DelayTime; //���ˑҋ@����
    float LifeTime; //��������
    float CountTime; //�o�ߎ���
    int isAlive; //�������Ă��邩
    int isWaiting; //�����ҋ@����
    int isInitialized;
    int ZAngle; //��]
    int RandNum; //�����_���ɐݒ肳��鐔�l
};

RWStructuredBuffer<ParticleUAVState> g_OutState : register(u0);

//�����_���Ȓl��Ԃ�
int fRandom(float x)
{
    return frac(sin(x) * 1000) * 1000;
}

//�p�[�e�B�N���̏�����
void ParticleInit(uint DTid_)
{
    float4 setQuot;
    float4 QtX, QtY, QtZ; //�w�莲��]�̃N�H�[�^�j�I��

    //�����_���Ȋp�x�̉�]���𐶐�
    QtX = CreateRotateQt(radians(fmod(fRandom(DTid_ + g_InState[0].iTime), g_InState[0].iAngleRange) + g_InState[0].iAngle.x), 1, 0, 0);
    QtY = CreateRotateQt(radians(fmod(fRandom(DTid_ + g_InState[0].iTime + 1), g_InState[0].iAngleRange) + g_InState[0].iAngle.y), 0, 1, 0);
    QtZ = CreateRotateQt(radians(fmod(fRandom(DTid_ + g_InState[0].iTime + 2), g_InState[0].iAngleRange) + g_InState[0].iAngle.z), 0, 0, 1);
    
    
    setQuot = MulQuot(MulQuot(QtX, QtY), QtZ); //����

    g_OutState[DTid_].m_Matrix = QttoMatrix(setQuot); //�s��ɕϊ����Ĕ��f

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

//�^�[�Q�b�g��ǂ�������p�[�e�B�N�����쏈��
//�ړ��̌������N�H�[�^�j�I���ŕύX������
void TargetChase(uint DTid_)
{
    //�s��S�̂��ύX�����̂ō��W��ۑ�����
    float3 Position = float3(g_OutState[DTid_].m_Matrix._14, g_OutState[DTid_].m_Matrix._24, g_OutState[DTid_].m_Matrix._34);
    float3 ZDir = float3(g_OutState[DTid_].m_Matrix._13, g_OutState[DTid_].m_Matrix._23, g_OutState[DTid_].m_Matrix._33);
    float3 TargetPos = /* -1.0f * */g_InState[0].iTargetPosition.xyz;
    float3 TargetVector = -1.0f * (TargetPos.xyz - Position.xyz); //float3(TargetPos.x - Position.x, TargetPos.y - Position.y, TargetPos.z - Position.z); //�����v�Z

    matrix Mat = transpose(g_OutState[DTid_].m_Matrix);//�s���]�u

    ZDir = normalize(ZDir);
    TargetVector = normalize(TargetVector);
    
    float4 Quoternion = CreateQtfromMatrix(g_OutState[DTid_].m_Matrix); //�s�񂩂�N�H�[�^�j�I���쐬
    float4 TransQuoternion = CreateQtfromMatrix(Mat);
    float4 TargetQuoternion;
    float Dot;

    Dot = dot(ZDir, TargetVector);
    
    Dot = min(Dot, 1.0f);
    Dot = max(Dot, -1.0f);

    TargetQuoternion = RotationArc(ZDir, TargetVector, Dot);
    float AngleDiff = acos(Dot);//���W�A���p�x
    float AngleMax = (3.141592741f) * g_InState[0].iMaxChaseAngle / 180.0f;
    float AngleMin = (3.141592741f) * g_InState[0].iMinChaseAngle / 180.0f;
    //�p��������
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

    //�N�H�[�^�j�I�����s��ɂ���
    g_OutState[DTid_].m_Matrix = QttoMatrix(Quoternion);

    //�ۑ����Ă������W�𔽉f
    g_OutState[DTid_].m_Matrix._14 = Position.x;
    g_OutState[DTid_].m_Matrix._24 = Position.y;
    g_OutState[DTid_].m_Matrix._34 = Position.z;

}

//�p�[�e�B�N���̍X�V
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
    //�ړ��v�Z
    g_OutState[DTid_].m_Matrix._14 += g_OutState[DTid_].m_Matrix._13 * Speed * g_InState[0].iTime;
    g_OutState[DTid_].m_Matrix._24 += g_OutState[DTid_].m_Matrix._23 * Speed * g_InState[0].iTime;
    g_OutState[DTid_].m_Matrix._34 += g_OutState[DTid_].m_Matrix._33 * Speed * g_InState[0].iTime;

    //��������
    g_OutState[DTid_].LifeTime -= g_InState[0].iTime;

    if (g_OutState[DTid_].LifeTime <= 0)
    {
        g_OutState[DTid_].isAlive = false;
    }

}

#endif