//--------------------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------------------
#include	"psvscommon.fx"

struct PS_OUTPUT
{
    float4 target0 : SV_TARGET0;
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
    return output;
}