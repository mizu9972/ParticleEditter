#pragma once

#include	<d3d11.h>
#include    <time.h>


//時間計測に関するクラス
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

	//現在時間を返す
	DWORD GetActuallyTimePoint() {
		DWORD NowTime = timeGetTime();
		return NowTime;
	}

	//経過時間を返す
	DWORD GetDurationTime(DWORD oldTime) {
		DWORD NowTime = timeGetTime();
		return NowTime - oldTime;
	}

};