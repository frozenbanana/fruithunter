#include "Apple.hpp"

void Apple::updateAnimated() {
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes
	float3 posOrder[] = {
		// Temporarely order of position. Later will be velocity for moving
		float3(0, 0, -4),
		float3(0, 0, -4),
		float3(0, 0, -4),
		float3(0, 3, 0),
		float3(0, 0, 4),
		float3(0, 0, 4),
	};
	float frameSpeedOrder[] = { 4, 6, 1.7, 1.7, 4, 2 };

	// Update frame time
	float currentClock = clock();
	float dt = (currentClock - m_clock) / 1000;
	m_clock = currentClock;
	m_frameTime += dt * frameSpeedOrder[m_currentFramePhase];

	// Maybe chaange keyframes
	if (m_frameTime > 1) {
		m_frameTime -= 1;
		m_currentFramePhase = (m_currentFramePhase + 1) % (m_nrOfFramePhases);
		m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
			frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)]);
	}

	// Set position
	float3 pos = XMVectorLerp(posOrder[m_currentFramePhase],
		posOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)], m_frameTime);
	setPosition(pos);

	// Update mesh specificly with our frametime
	m_meshAnim.update(m_frameTime);
}

Apple::Apple() : Entity() {
	m_nrOfFramePhases = 6;
	m_currentFramePhase = 0;
	m_frameTime = 0.0f;
	m_clock = clock();
	loadAnimated("Bouncing_apple", 3);
}
