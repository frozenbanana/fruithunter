#include "Apple.hpp"

void Apple::updateAnimated(float dt) {
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes
	float3 posOrder[] = {
		// Temporarely order of position. Later will be velocity for moving
		float3(0.f, 0.f, -4.f),
		float3(0.f, 0.f, -4.f),
		float3(0.f, 0.f, -4.f),
		float3(0.f, 3.f, 0.f),
		float3(0.f, 0.f, 4.f),
		float3(0.f, 0.f, 4.f),
	};
	float frameSpeedOrder[] = { 4.f, 6.f, 1.7f, 1.7f, 4.f, 2.f };

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
	m_meshAnim.updateSpecific(m_frameTime);
}

Apple::Apple() : Entity() {
	m_nrOfFramePhases = 6;
	m_currentFramePhase = 0;
	m_frameTime = 0.0f;
	loadAnimated("Bouncing_apple", 3);
}
