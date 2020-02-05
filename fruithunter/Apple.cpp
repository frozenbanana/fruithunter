#include "Apple.hpp"

void Apple::updateAnimated() {
	int frameOrder[] = { 0, 1, 2, 0, 1 };
	float currentClock = clock();
	float dt = (currentClock - m_clock) / 1000;
	m_clock = currentClock;
	m_frameTime += dt;
	if (m_frameTime > 1) {
		m_frameTime -= 1;
		m_currentFramePhase = (m_currentFramePhase + 1) % (m_nrOfFramePhases - 1);
		m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
			frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases - 1)]);
	}
	m_meshAnim.update(m_frameTime);
}

Apple::Apple() : Entity() {
	m_nrOfFramePhases = 5;
	m_currentFramePhase = 0;
	m_frameTime = 0.0f;
	m_clock = clock();
	loadAnimated("Bouncing_apple", 3);
}
