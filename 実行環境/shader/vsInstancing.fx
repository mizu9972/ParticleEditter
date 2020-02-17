//--------------------------------------------------------------------------------------
// vs.fx
//--------------------------------------------------------------------------------------
#include	"psvscommon.fx"

//入力パラメータ
struct VS_INSTANCING_IN
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
    float2 Tex : TEXCOORD;
    column_major float4x4 Mat : MATRIX;
    uint InstanceID : SV_InstanceID;
};

//出力パラメータ
struct VS_INSTANCING_OUT
{
    float4 SV_Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float3 Tex : TEXCOORD;
};
//--------------------------------------------------------------------------------------
// 頂点シェーダー
//--------------------------------------------------------------------------------------
VS_INSTANCING_OUT main(VS_INSTANCING_IN input)
{
    VS_INSTANCING_OUT output = (VS_INSTANCING_OUT) 0;

    //output.SV_Pos = mul(float4(input.Pos, 1.0f), input.Mat);
    //output.Normal = input.Normal;
    //output.Tex = float3(input.Tex, input.InstanceID);

    float4x4 mtx;
    mtx = mul(input.Mat, World);
    
    output.SV_Pos = mul(input.Pos, mtx);
    output.SV_Pos = mul(output.SV_Pos, View);
    output.SV_Pos = mul(output.SV_Pos, Projection);
    output.Tex = input.Tex;
    
    float4 N = input.Normal;
    N.w = 0;
    N = mul(N, mtx);
    N = normalize(N);
    
    output.Normal = N;
    
	return output;
}