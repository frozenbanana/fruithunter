#include "Apple.h"
#include "Input.h"

Apple::Apple(float3 pos) : Fruit(pos) {
	loadAnimated("Bouncing_apple", 3);
	m_nrOfFramePhases = 6;
	setScale(0.5);
	changeState(AI::State::PASSIVE);
	m_direction = float3((float)(rand() % 1), 0.0f, (float)(rand() % 1));
	m_velocity = float3(1.f);
	m_direction.Normalize();
}

void Apple::updateAnimated(float dt) {
	m_startPos = m_position;
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes
	float3 posOrder[6] = {
		m_startPos, m_startPos, m_startPos, m_startPos, m_startPos, m_startPos,
		/*m_heightPos,
		m_destinationPos
		m_destinationPos,*/
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

void Apple::move(float dt) {
	if (!m_availablePath.empty() && m_currentState == ACTIVE) {
		m_direction = (m_availablePath.back() - m_position);
		m_availablePath.pop_back();
	}

	m_direction.y = 0.f;
	m_position += m_direction * dt;
	setPosition(m_position);
}

void Apple::update(float dt, float3 playerPosition, float height) {
	m_position.y = height;
	float distanceToPlayer = (playerPosition - m_position).Length();

	if (distanceToPlayer < 7.f && m_currentState == PASSIVE) {
		flee(playerPosition); // updates path
		changeState(AI::ACTIVE);
	}
	else if (m_availablePath.empty()) { // is path finished set pasive state
		AI::changeState(AI::PASSIVE);
		m_direction = float3(sin(dt), 0.0f, (float)(rand() % 5));
		m_direction.Normalize();
	}
	updateAnimated(dt);
	move(dt);
}

void Apple::flee(float3 playerPos) {
	float3 start = float3(m_startPos.x, 0.0, m_startPos.z);
	float3 end = float3(playerPos.x, 0.0f, playerPos.z);
	pathfinding(start, end);
}
