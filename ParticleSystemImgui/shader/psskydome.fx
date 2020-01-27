//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
#include	"psvscommon.fx"

struct PS_OUTPUT
{
    float4 target0 : SV_TARGET0;
    float2 target1 : SV_TARGET1;
};
//--------------------------------------------------------------------------------------
//  ピクセルシェーダー
//--------------------------------------------------------------------------------------
PS_OUTPUT main(VS_OUTPUT input)
{
    PS_OUTPUT output;
	float4 texcol = g_Tex.Sample(g_SamplerLinear, input.Tex);
	float4 col = texcol;
    
    output.target0 = col;
    output.target1 = input.Pos.zw;
    //output.target1 = 1.0f;
    return output;
}