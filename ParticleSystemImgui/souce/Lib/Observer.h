#pragma once
#include <vector>
//別クラスを監視するオブザーバパターンクラスファイル
class Subject;
class  Observer abstract {
private:
public:
	Observer() {};
	virtual ~Observer() {};
	virtual void OnNotify(Subject* subject_ = nullptr) {};
};

class Subject abstract {
	//#編集不要
private:
	std::vector<Observer*> m_ObserverList;
public:
	void AddObsever(Observer* obsever_);
	void RemoveObserver(Observer* observer_);
protected:
	void Notify();
	void Notify(Subject* subject_);
};