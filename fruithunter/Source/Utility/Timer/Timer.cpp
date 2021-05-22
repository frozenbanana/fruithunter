#include "Timer.h"
#include "ErrorLogger.h"
#include <string>

Timer::Timer() { reset(); }

void Timer::update() {
	clock_t time = clock();
	m_time_frame = time - m_timer;
	// Returns 0 if dt is too big. Fixes problem with accumulated time from changing states
	if (m_time_frame > 1000)
		m_time_frame = 0.f;
	m_time_total += m_time_frame;
	m_timer = time;
}

double Timer::getTimePassed() { return (double)m_time_total / MILLISECONDS_IN_SECONDS; }

time_t Timer::getTimePassedAsMilliseconds() const { return (time_t)m_time_total; }

void Timer::reset() {
	m_timer = clock();
	m_time_start = m_timer;
	m_time_total = 0.f;
	m_time_frame = 0.f;
}


double Timer::getDt() { return (double)m_time_frame / MILLISECONDS_IN_SECONDS; }

std::string Timer::getTimeToString() { return std::to_string(getTimePassed()); }