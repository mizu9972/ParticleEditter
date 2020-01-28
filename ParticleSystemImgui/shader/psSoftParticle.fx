//--------------------------------------------------------------------------------------
// ps.fx
//--------------------------------------------------------------------------------------
#include	"psvscommon.fx"

Texture2D g_DepthMap : register(t1);//深度マップ

cbuffer ConstantBufferParticle : register(b5)
{
    int2 iViewPort;
    float iZfar;
    float iZVolume;

}

//--------------------------------------------------------------------------------------
//  ソフトパーティクル用ピクセルシェーダー
//--------------------------------------------------------------------------------------
float4 main(VS_OUTPUT input) : SV_Target
{
    float4 texcol = g_Tex.Sample(g_SamplerLinear, input.Tex);
    float4 col;
        
    col.xyz = texcol.xyz * input.Color.xyz;
    col.w = texcol.w * input.Color.w;
    
    float2 uv;
    uv.x = input.Pos.x / iViewPort.x;
    uv.y = input.Pos.y / iViewPort.y;
    uv.xy = uv.xy;
    
    float2 depth = g_DepthMap.Sample(g_SamplerLinear, uv).xy;

    float a = 1.0f;
    
    //後ろ側にあるパーティクルのピクセルは描画しない
    if (input.Pos.z > depth.x)
    {
        discard;
    }
    
    //後ろにオブジェクトがない場合はそのまま描画
    if (depth.x == 1.0f)
    {
        return float4(col.r, col.g, col.b, col.a);

    }
    
    float Zsub = (depth.x - input.Pos.z) * 100.0f; //深度値の差
    
    if (Zsub < iZfar / input.Pos.z * 10.0f)
    {
        a = Zsub * iZVolume * (input.Pos.z);
    }
    
    return float4(col.r, col.g, col.b, col.a * a);
}