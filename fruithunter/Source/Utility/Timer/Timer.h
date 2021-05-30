#pragma once
#include "GlobalNamespaces.h"
#include <time.h>
class Timer {
public:
	Timer();
	void update();
	double getTimePassed();
	time_t getTimePassedAsMilliseconds() const;
	string getTimeToString();
	double getDt();
	void reset();

private:
	clock_t m_timer;
	const clock_t MILLISECONDS_IN_SECONDS = 1000;
	clock_t m_time_start;
	clock_t m_time_total;
	clock_t m_time_frame;
};
