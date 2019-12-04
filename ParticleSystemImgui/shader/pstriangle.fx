struct VS_OUTPUT {
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};
Texture2D g_Tex : register(t0);					// テクスチャ
SamplerState g_SamplerLinear : register(s0);	// サンプラー

float4 main( VS_OUTPUT input ) : SV_Target
{
	float4 texcol;

	// テクスチャからテクセルを取得する
	texcol = g_Tex.Sample(
		g_SamplerLinear,						// サンプラー 
		input.Tex);								// テクスチャ座標

	return texcol;
}
