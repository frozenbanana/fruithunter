#include "Timer.hpp"
#include "ErrorLogger.hpp"
#include <string>

Timer::Timer() {
	LARGE_INTEGER m_timer;
	if (!QueryPerformanceCounter(&m_timer)) {
		ErrorLogger::log("Cannot query performance counter in timer.");
		return;
	}
	m_frequencySeconds = (float)(m_timer.QuadPart);
	// Get Current value
	QueryPerformanceCounter(&m_timer);
	m_startTime = m_timer.QuadPart;
	m_totalTime = 0.;
	m_elapsedTime = 0.;
}

void Timer::update() {
	QueryPerformanceCounter(&m_timer);
	m_lastElapsedTime = m_elapsedTime;
	m_elapsedTime = (float)(m_timer.QuadPart - m_startTime);
	m_startTime = m_timer.QuadPart;
	// hard coded value perhaps could be replaced with m_frequencySeconds
	m_totalTime += m_elapsedTime * 0.000001;
}

float Timer::getTime() { return m_totalTime; }

float Timer::getDt() { return (m_elapsedTime - m_lastElapsedTime) * 0.000001; }

std::string Timer::getTimeToString() { return std::to_string(m_totalTime); }