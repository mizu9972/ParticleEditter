#pragma once
#include <vector>
//�ʃN���X���Ď�����I�u�U�[�o�p�^�[���N���X�t�@�C��
class Subject;
class  Observer abstract {
private:
public:
	Observer() {};
	virtual ~Observer() {};
	virtual void OnNotify(Subject* subject_ = nullptr) {};
};

class Subject abstract {
	//#�ҏW�s�v
private:
	std::vector<Observer*> m_ObserverList;
public:
	void AddObsever(Observer* obsever_);
	void RemoveObserver(Observer* observer_);
protected:
	void Notify();
	void Notify(Subject* subject_);
};