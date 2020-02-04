#pragma once
#include "CParticle.h"
#include "CModel.h"

//パーティクルの描画をXファイルのモデルで行う場合に利用する
class XFileParticleSystem : public ParticleSystem{
private:
	CModel m_Model;

public:
	void ModelInit(const char* filename, const char* vsfile, const char* psfile);
	virtual void Draw();
};