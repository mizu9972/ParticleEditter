//--------------------------------------------------------------------------------------
// texvs.fx
//--------------------------------------------------------------------------------------
struct VS_IN {
	float4 pos:POSITION0;
	float4 nor:NORMAL0;
	float2 tex:TEXCOORD0;
};

struct VS_OUT {
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

cbuffer ConstantBuffer {
	float4x4 world;
	float4x4 view;
	float4x4 projection;
};
//--------------------------------------------------------------------------------------
// 頂点シェーダー
//--------------------------------------------------------------------------------------
VS_OUT main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	output.pos = mul(input.pos, world);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);

	output.tex = input.tex;

	return output;
}