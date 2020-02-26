#pragma once
#include "GlobalNamespaces.h"
#include <time.h>
class Timer {
public:
	Timer();
	void update();
	float getTimePassed();
	string getTimeToString();
	float getDt();
	void reset();

private:
	clock_t m_timer;
	float m_frequencySeconds;
	float m_startTime;
	float m_totalTime;
	float m_elapsedTime;
};
