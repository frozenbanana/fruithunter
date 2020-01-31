#pragma once
#include <d3d11.h>

class Timer {
public:
	Timer();
	void update();
	float getTime();
	float getDt();

private:
	LARGE_INTEGER m_timer;
	float m_frequencySeconds;
	float m_startTime;
	float m_totalTime;
	float m_elapsedTime;
	float m_lastElapsedTime;
};
