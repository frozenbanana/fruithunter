#include "Apple.hpp"
#include "Input.hpp"
void Apple::updateAnimated(float dt) {
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes
	float3 posOrder[6] = {
		// Temporarely order of position. Later will be velocity for moving
		m_startPos,
		m_startPos,
		m_startPos,
		m_heightPos,
		m_destinationPos,
		m_destinationPos,
	};
	bool justChanged = false;
	float frameSpeedOrder[] = { 4.f, 5.f, 2.0f, 1.9f, 4.f, 2.f };
	m_frameTime += dt * frameSpeedOrder[m_currentFramePhase];
	if (Input::getInstance()->keyDown(Keyboard::K))
		int th = 0;
	// Maybe change keyframes
	if (m_frameTime > 1) {
		m_frameTime -= 1;
		m_currentFramePhase = m_currentFramePhase + 1;

		if (m_currentFramePhase == m_nrOfFramePhases) {
			m_currentFramePhase = 0;
			setDestination();
			justChanged = true;
			setRotation(float3(0.f, findRequiredRotation(m_nextDestinationPos), 0.f));
		}

		m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
			frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)]);
	}
	if (m_currentFramePhase >= 2 || justChanged) {
		posOrder[0] = getPosition();
		posOrder[1] = getPosition();
	}
	// Set position
	float3 pos = XMVectorLerp(posOrder[m_currentFramePhase],
		posOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)], m_frameTime);
	setPosition(pos);

	// Update mesh specificly with our frametime
	m_meshAnim.updateSpecific(m_frameTime);
}

void Apple::setNextDestination(float3 nextDest) { m_nextDestinationPos = nextDest; }

float Apple::findRequiredRotation(float3 lookAt) {
	float rot = 0.f;

	float dx = lookAt.x - getPosition().x;
	float dz = lookAt.z - getPosition().z;
	if (dx != 0)
		rot = -atan(dz / dx);
	else
		rot = 0;

	if (dx < 0)
		rot = 3.1416f + rot;
	return rot + 3.14f * 0.5f;
}

void Apple::setDestination() {
	m_destinationPos = m_nextDestinationPos;
	m_startPos = getPosition();
	m_heightPos = XMVectorLerp(m_startPos, m_destinationPos, 0.5f);
	m_heightPos.y += 1.f;
}

Apple::Apple() : Entity() {
	m_nrOfFramePhases = 6;
	m_currentFramePhase = 0;
	m_frameTime = 0.0f;
	loadAnimated("Bouncing_apple", 3);
	m_startPos = float3(0.f, 0.f, 0.f);
	m_heightPos = float3(0.f, 0.f, 0.f);
	m_destinationPos = float3(0.f, 0.f, 0.f);
	m_nextDestinationPos = float3(0.f, 0.f, 0.f);
}
