//--------------------------------------------------------------------------------------
// ps.fx
//--------------------------------------------------------------------------------------
#include	"psvscommon.fx"

Texture2D g_DepthMap : register(t1);//�[�x�}�b�v

cbuffer ConstantBufferParticle : register(b5)
{

    Matrix Mat; //�s��
    int ZAngle;
    float CountTime;

    float RandNum;

    int RotateSpeed; //��]���x
    float Speed; //���x
    float Color[4]; //�p�[�e�B�N���̐F

}

//--------------------------------------------------------------------------------------
//  �\�t�g�p�[�e�B�N���p�s�N�Z���V�F�[�_�[
//--------------------------------------------------------------------------------------
float4 main(VS_OUTPUT input) : SV_Target
{
    float4 texcol = g_Tex.Sample(g_SamplerLinear, input.Tex);
    float4 col;
    
    float2 uv = input.Pos.xy / input.Pos.w;
    uv.x = input.Pos.x / 1600.0f;
    uv.y = input.Pos.y / 900.0f;
    
    float2 DepthCoord;
    DepthCoord.x = (input.Pos.x - 1);
    DepthCoord.y = (1 - input.Pos.y) * 0.5f;
    float2 depth = g_DepthMap.Sample(g_SamplerLinear, uv).xy;
    
    
    col.xyz = texcol.xyz * input.Color.xyz;
    col.w = texcol.w * input.Color.w;
    
    float a = 1.0f;
    
    if (depth.x > input.Pos.z)
    {
        a = 0;
    }
    else
    {
        a = 1.0f;
    }
    
    float Zfar;//�t�F�[�h�J�n����
    float Volume;//�t�F�[�h�̋���
    
    
    
    return float4(1.0f, 1.0f, 1.0f, a);
    //float l = depth.x / 100.0f - (input.Pos.z / 100.0f - col.a * 0.03f);
    

    
    //if (depth.x != 0.0f)
    //{
    //    a = 0.0f;
    //}
    
    //return float4(1.0f, 1.0f, a, 1.0f);
    //if (l <= 0)
    //{
    //    discard;
    //}
    //if (l < 0.05f)
    //{
    //    a = max(l / 0.05f, 0.0f);

    //}
    
    //return float4(col.rgb, col.a * a);
}