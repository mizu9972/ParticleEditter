//--------------------------------------------------------------------------------------
// ps.fx
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
	float4 N = input.Normal;
	float4 L = LightDirection;
	N.w = 0.0f;					// 法線はベクトルなのでＷの値を０にする。
	L.w = 0.0f;					// 光の方向はベクトルなのでＷの値を０にする。

	N = normalize(N);			// 法線ベクトル正規化
	L = normalize(L);			// 光の方向ベクトル正規化

	// 拡散反射光の計算
	float d = max(0.0, dot(L, N));			// ランバート余弦則
	float4 diffuse = diffuseMaterial * d;	// マテリアル値と掛け算

	// 鏡面反射光の計算（ブリンフォン）
	float4 H;
	float4 V = normalize(EyePos - input.WPos);
	H = normalize(L + V);

	float s = max(0, dot(N, H));
	s = pow(s, 50);
	float4 specular = s * specularMaterial;

	float4 texcol = g_Tex.Sample(g_SamplerLinear, input.Tex);
    PS_OUTPUT col;
	col.target0 = specular + diffuse * texcol;
    col.target0.a = 1.0f;
    
    col.target1 = input.Pos.zw;
	return col;
}