
// 定数バッファ構造体
cbuffer ConstantBufferColor : register(b1)
{
	float4 color;
}

float4 main() : SV_Target
{
	return color;
}