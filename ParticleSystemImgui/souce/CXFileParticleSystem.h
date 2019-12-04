#pragma once
#include "CParticle.h"
#include "CModel.h"

//�p�[�e�B�N���̕`���X�t�@�C���̃��f���ōs���ꍇ�ɗ��p����
class XFileParticleSystem : public ParticleSystem{
private:
	CModel m_Model;

public:
	void ModelInit(const char* filename, const char* vsfile, const char* psfile);
	virtual void Draw();
};