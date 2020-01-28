//--------------------------------------------------------------------------------------
// ps.fx
//--------------------------------------------------------------------------------------
#include	"psvscommon.fx"

Texture2D g_DepthMap : register(t1);//�[�x�}�b�v

cbuffer ConstantBufferParticle : register(b5)
{
    int2 iViewPort;
    float iZfar;
    float iZVolume;

}

//--------------------------------------------------------------------------------------
//  �\�t�g�p�[�e�B�N���p�s�N�Z���V�F�[�_�[
//--------------------------------------------------------------------------------------
float4 main(VS_OUTPUT input) : SV_Target
{
    float4 texcol = g_Tex.Sample(g_SamplerLinear, input.Tex);
    float4 col;
        
    col.xyz = texcol.xyz * input.Color.xyz;
    col.w = texcol.w * input.Color.w;
    
    float2 uv;
    uv.x = input.Pos.x / iViewPort.x;
    uv.y = input.Pos.y / iViewPort.y;
    uv.xy = uv.xy;
    
    float2 depth = g_DepthMap.Sample(g_SamplerLinear, uv).xy;

    float a = 1.0f;
    
    //��둤�ɂ���p�[�e�B�N���̃s�N�Z���͕`�悵�Ȃ�
    if (input.Pos.z > depth.x)
    {
        discard;
    }
    
    //���ɃI�u�W�F�N�g���Ȃ��ꍇ�͂��̂܂ܕ`��
    if (depth.x == 1.0f)
    {
        return float4(col.r, col.g, col.b, col.a);

    }
    
    float Zsub = (depth.x - input.Pos.z) * 100.0f; //�[�x�l�̍�
    
    if (Zsub < iZfar / input.Pos.z * 10.0f)
    {
        a = Zsub * iZVolume * (input.Pos.z);
    }
    
    return float4(col.r, col.g, col.b, col.a * a);
}