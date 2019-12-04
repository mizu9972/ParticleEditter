struct VS_OUTPUT {
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};
Texture2D g_Tex : register(t0);					// �e�N�X�`��
SamplerState g_SamplerLinear : register(s0);	// �T���v���[

float4 main( VS_OUTPUT input ) : SV_Target
{
	float4 texcol;

	// �e�N�X�`������e�N�Z�����擾����
	texcol = g_Tex.Sample(
		g_SamplerLinear,						// �T���v���[ 
		input.Tex);								// �e�N�X�`�����W

	return texcol;
}
