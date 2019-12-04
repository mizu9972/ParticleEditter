//--------------------------------------------------------------------------------------
// ps.fx
//--------------------------------------------------------------------------------------
#include	"psvscommon.fx"

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
//  �s�N�Z���V�F�[�_�[
//--------------------------------------------------------------------------------------
float4 main(VS_OUTPUT input) : SV_Target
{
    float4 texcol = g_Tex.Sample(g_SamplerLinear, input.Tex);
    float4 col;
    col.xyz = texcol.xyz * input.Color.xyz;
//	col.a = input.Color.a;
    col.w = texcol.w * input.Color.w;
    return col;
}