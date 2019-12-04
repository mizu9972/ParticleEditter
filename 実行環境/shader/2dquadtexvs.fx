#include	"psvscommon2.fx"

//--------------------------------------------------------------------------------------
// ���_�V�F�[�_�[
//--------------------------------------------------------------------------------------
VS_OUTPUT main(	float4 Pos		: POSITION, 
				float4 Color	: COLOR,
				float2 Tex		: TEXCOORD)
{

	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Pos = mul(Pos, World);			// ���[���h�ϊ�

	output.Color = Color;					// �J���[

	// �X�N���[�����W�ɕϊ�
	output.Pos.x = (output.Pos.x / ViewportWidth.x) * 2 - 1.0f;
	output.Pos.y = 1.0f-(output.Pos.y / ViewportHeight.x) * 2;
	output.Pos.z = 0.0f;

	// �e�N�X�`�����W
	output.Tex.x = Tex.x;
	output.Tex.y = Tex.y;

	return output;
}
