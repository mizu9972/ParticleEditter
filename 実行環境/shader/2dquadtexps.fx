#include	"psvscommon2.fx"

//--------------------------------------------------------------------------------------
//  ピクセルシェーダー
//--------------------------------------------------------------------------------------
float4 main( VS_OUTPUT input ) : SV_Target
{
	float4 texcol = g_Tex.Sample(g_SamplerLinear, input.Tex);
	texcol.r = input.Color.r;
	texcol.g = input.Color.g;
	texcol.b = input.Color.b;
	return texcol;
}