#include	"psvscommon2.fx"

//--------------------------------------------------------------------------------------
// 頂点シェーダー
//--------------------------------------------------------------------------------------
VS_OUTPUT main(	float4 Pos		: POSITION, 
				float4 Color	: COLOR,
				float2 Tex		: TEXCOORD)
{

	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Pos = mul(Pos, World);			// ワールド変換

	output.Color = Color;					// カラー

	// スクリーン座標に変換
	output.Pos.x = (output.Pos.x / ViewportWidth.x) * 2 - 1.0f;
	output.Pos.y = 1.0f-(output.Pos.y / ViewportHeight.x) * 2;
	output.Pos.z = 0.0f;

	// テクスチャ座標
	output.Tex.x = Tex.x;
	output.Tex.y = Tex.y;

	return output;
}
