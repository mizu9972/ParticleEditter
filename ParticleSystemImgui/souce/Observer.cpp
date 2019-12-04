#include "Observer.h"

//-----------------------------------------------
void Subject::Notify() {
	//�ʒm
	for (unsigned int ObserverNum = 0; ObserverNum < m_ObserverList.size(); ObserverNum++) {
		m_ObserverList[ObserverNum]->OnNotify();
	}
}

void Subject::Notify(Subject* subject_) {
	for (unsigned int ObserverNum = 0; ObserverNum < m_ObserverList.size(); ObserverNum++) {
		m_ObserverList[ObserverNum]->OnNotify(subject_);
	}
}

void Subject::AddObsever(Observer* observer_) {
	//�I�u�U�[�o�ǉ�
	m_ObserverList.emplace_back(observer_);
}

void Subject::RemoveObserver(Observer* observer_) {
	//�I�u�U�[�o�폜
	for (unsigned int ObserverNum = 0; m_ObserverList.size(); ObserverNum++) {
		if (m_ObserverList[ObserverNum] == observer_) {
			m_ObserverList.erase(m_ObserverList.begin() + ObserverNum);
			m_ObserverList.shrink_to_fit();

			break;
		}
	}
}
//-----------------------------------------------