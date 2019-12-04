#pragma once
#include <vector>
//別クラスを監視するオブザーバパターンクラスファイル
class Subject;
class Observer {
private:
public:
	Observer() {};
	virtual ~Observer() {};
	virtual void OnNotify() {};
	virtual void OnNotify(Subject* subject_) {};
};

class Subject {
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