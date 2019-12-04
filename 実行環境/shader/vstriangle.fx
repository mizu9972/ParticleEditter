struct VS_OUTPUT {
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};
cbuffer ConstantBufferWorld : register(b1)
{
	matrix	World;					// ���[���h�ϊ��s��
}

cbuffer ConstantBufferViewPort : register(b2)
{
	uint4	ViewportWidth;			// �r���[�|�[�g��
	uint4	ViewportHeight;			// �r���[�|�[�g����
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main( 
	float4 Pos : POSITION,
	float4 Tex : TEXCOORD)
{
	VS_OUTPUT o;

	// �[���N���A
	o = (VS_OUTPUT)0;

	// ���[���h�ϊ�
	o.Pos = mul(Pos,World);

	// �X�N���[�����W�ɕϊ�
	o.Pos.x = (o.Pos.x / ViewportWidth.x) * 2.0f - 1.0f;
	o.Pos.y = 1.0f - (o.Pos.y / ViewportHeight.x) * 2.0f;
	o.Pos.z = 0.0f;

	// �e�N�X�`�����W���Z�b�g
	o.Tex.x = Tex.x;
	o.Tex.y = Tex.y;

	return o;
}
