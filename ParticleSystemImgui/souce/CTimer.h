#pragma once

#include	<d3d11.h>
#include    <time.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

float g_SectionTime;
//時間計測に関するクラス
class CTimer {
private:
	DWORD StartTime;
	DWORD m_time;
public:
	CTimer() {
		//timeBeginPeriod(1);
		StartTime = timeGetTime();
	}
	~CTimer() {
		m_time = timeGetTime();
		g_SectionTime = (float)((m_time - StartTime) * 0.001f);


	}
	
};