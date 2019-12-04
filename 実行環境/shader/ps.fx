//--------------------------------------------------------------------------------------
// ps.fx
//--------------------------------------------------------------------------------------
#include	"psvscommon.fx"

//--------------------------------------------------------------------------------------
//  ピクセルシェーダー
//--------------------------------------------------------------------------------------
float4 main(VS_OUTPUT input) : SV_Target
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
	float4 col;
	col = specular + diffuse * texcol;
	col.a = 1.0f;
	return col;
}