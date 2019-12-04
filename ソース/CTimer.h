#pragma once

#include	<d3d11.h>
#include    <time.h>


//���Ԍv���Ɋւ���N���X
class CTimer {
private:
	DWORD StartTime;
	CTimer() { 
		timeBeginPeriod(1);
		StartTime = timeGetTime(); };
	~CTimer() = default;
public:
	CTimer(const CTimer&) = delete;
	CTimer& operator=(const CTimer&) = delete;
	CTimer(CTimer&&) = delete;
	CTimer& operator = (CTimer&&) = delete;

	static CTimer* GetInstance() {
		static CTimer instance;
		return &instance;
	}

	//���ݎ��Ԃ�Ԃ�
	DWORD GetActuallyTimePoint() {
		DWORD NowTime = timeGetTime();
		return NowTime;
	}

	//�o�ߎ��Ԃ�Ԃ�
	DWORD GetDurationTime(DWORD oldTime) {
		DWORD NowTime = timeGetTime();
		return NowTime - oldTime;
	}

};