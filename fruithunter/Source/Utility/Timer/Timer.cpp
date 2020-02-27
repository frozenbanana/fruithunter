#include "Timer.h"
#include "ErrorLogger.h"
#include <string>

Timer::Timer() {
	m_frequencySeconds = 1000;
	// Get Current value
	m_timer = clock();
	m_startTime = (float)m_timer;
	m_totalTime = 0.f;
	m_elapsedTime = 0.f;
}

void Timer::update() {
	clock_t time = clock();
	m_elapsedTime = (time - m_timer) / m_frequencySeconds;
	// Returns 0 if dt is too big. Fixes problem with accumulated time from changing states
	if (m_elapsedTime > 1.f)
		m_elapsedTime = 0.f;
	m_timer = time;
	m_totalTime += m_elapsedTime;
}

float Timer::getTimePassed() { return m_totalTime; }

void Timer::reset() {
	m_timer = clock();
	m_startTime = (float)m_timer;
	m_totalTime = 0.f;
	m_elapsedTime = 0.f;
}


float Timer::getDt() { return m_elapsedTime; }

std::string Timer::getTimeToString() { return std::to_string(m_totalTime); }