#pragma once
#include	<d3d11.h>
namespace ParticleSystemUtility {
	bool CreateConstantBuffer(ID3D11Device* device, unsigned int bytesize, ID3D11Buffer** pConstantBuffer);
}