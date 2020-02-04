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
//  �s�N�Z���V�F�[�_�[
//--------------------------------------------------------------------------------------
PS_OUTPUT main(VS_OUTPUT input)
{
	float4 N = input.Normal;
	float4 L = LightDirection;
	N.w = 0.0f;					// �@���̓x�N�g���Ȃ̂łv�̒l���O�ɂ���B
	L.w = 0.0f;					// ���̕����̓x�N�g���Ȃ̂łv�̒l���O�ɂ���B

	N = normalize(N);			// �@���x�N�g�����K��
	L = normalize(L);			// ���̕����x�N�g�����K��

	// �g�U���ˌ��̌v�Z
	float d = max(0.0, dot(L, N));			// �����o�[�g�]����
	float4 diffuse = diffuseMaterial * d;	// �}�e���A���l�Ɗ|���Z

	// ���ʔ��ˌ��̌v�Z�i�u�����t�H���j
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