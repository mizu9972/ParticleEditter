#include "CXFileParticleSystem.h"
#include "dx11mathutil.h"
#include "DX11Settransform.h"

void XFileParticleSystem::ModelInit(const char* filename, const char* vsfile, const char* psfile) {
	m_Model.Init(filename, vsfile, psfile);
}

void XFileParticleSystem::Draw() {
	for (int ParticlesNum = 0; ParticlesNum < m_ParticleState.m_ParticleNum; ParticlesNum++) {
		if (Particles[ParticlesNum].isAlive == false) {
			continue;
		}
		//s—ñ”½‰f
		DX11SetTransform::GetInstance()->SetTransform(DX11SetTransform::TYPE::WORLD, Particles[ParticlesNum].Matrix);
		//•`‰æ
		m_Model.Draw();
	}
}