//--------------------------------------------------------------------------------------
// ps.fx
//--------------------------------------------------------------------------------------
#include	"psvscommon.fx"

cbuffer ConstantBufferParticle : register(b5)
{

    Matrix Mat; //行列
    int ZAngle;
    float CountTime;

    float RandNum;

    int RotateSpeed; //回転速度
    float Speed; //速度
    float Color[4]; //パーティクルの色

}
//--------------------------------------------------------------------------------------
//  ピクセルシェーダー
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