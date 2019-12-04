struct VS_OUTPUT {
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};
cbuffer ConstantBufferWorld : register(b1)
{
	matrix	World;					// ワールド変換行列
}

cbuffer ConstantBufferViewPort : register(b2)
{
	uint4	ViewportWidth;			// ビューポート幅
	uint4	ViewportHeight;			// ビューポート高さ
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main( 
	float4 Pos : POSITION,
	float4 Tex : TEXCOORD)
{
	VS_OUTPUT o;

	// ゼロクリア
	o = (VS_OUTPUT)0;

	// ワールド変換
	o.Pos = mul(Pos,World);

	// スクリーン座標に変換
	o.Pos.x = (o.Pos.x / ViewportWidth.x) * 2.0f - 1.0f;
	o.Pos.y = 1.0f - (o.Pos.y / ViewportHeight.x) * 2.0f;
	o.Pos.z = 0.0f;

	// テクスチャ座標をセット
	o.Tex.x = Tex.x;
	o.Tex.y = Tex.y;

	return o;
}
