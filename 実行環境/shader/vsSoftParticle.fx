//--------------------------------------------------------------------------------------
// vs.fx
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
// 頂点シェーダー
//--------------------------------------------------------------------------------------
VS_OUTPUT main(	float4 Pos		:	POSITION,
				float4 Color	:	COLOR,
				float2 Tex		:	TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Pos = mul(Pos, World);
	output.WPos = output.Pos;
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Tex = Tex;

	output.Color = Color;

	return output;
}