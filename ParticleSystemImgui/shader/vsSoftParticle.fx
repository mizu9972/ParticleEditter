//--------------------------------------------------------------------------------------
// vs.fx
//--------------------------------------------------------------------------------------
#include	"psvscommon.fx"

cbuffer ConstantBufferParticle : register(b5)
{

    Matrix Mat; //�s��
    int ZAngle;
    float CountTime;

    float RandNum;

    int RotateSpeed; //��]���x
    float Speed; //���x
    float Color[4]; //�p�[�e�B�N���̐F

}
//--------------------------------------------------------------------------------------
// ���_�V�F�[�_�[
//--------------------------------------------------------------------------------------
VS_OUTPUT main(	float4 Pos		:	POSITION,
				float4 Color	:	COLOR,
				float2 Tex		:	TEXCOORD)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Pos = mul(Pos, World);
	output.WPos = output.Pos;
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Tex = Tex;

	output.Color = Color;

	return output;
}