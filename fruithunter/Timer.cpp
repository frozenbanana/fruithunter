#include "Timer.hpp"
#include "ErrorLogger.hpp"
#include <string>

Timer::Timer() {
	LARGE_INTEGER m_timer;
	if (!QueryPerformanceFrequency(&m_timer)) {
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
	m_elapsedTime = (float)(m_timer.QuadPart - m_startTime) / m_frequencySeconds;
	m_startTime = m_timer.QuadPart;
	m_totalTime += m_elapsedTime;
}

float Timer::getTimePassed() { return m_totalTime; }

float Timer::getDt() { return m_elapsedTime; }

std::string Timer::getTimeToString() { return std::to_string(m_totalTime); }