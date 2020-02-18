#include "Apple.h"
#include "Input.h"

Apple::Apple(float3 pos) : Fruit(pos) {
	loadAnimated("Bouncing_apple", 3);
	m_nrOfFramePhases = 6;
	setScale(0.5);
	changeState(AI::State::PASSIVE);

	m_directionalVelocity = float3(1.f, 0.f, 1.f);
	m_fruitType = APPLE;

	m_activationRadius = 3.f;
	m_passiveRadius = 6.f;
}

void Apple::behaviorPassive(float3 playerPosition) {
	ErrorLogger::log("Apple:: Doing passive.");
	ErrorLogger::logFloat3("m_position", m_position);
	float terrainHeight = TerrainManager::getInstance()->getHeightFromPosition(m_position);
	if (!withinDistanceTo(m_worldHome, 0.75f) && atOrUnder(terrainHeight)) {
		m_directionalVelocity = m_worldHome - m_position;
		m_directionalVelocity.Normalize();
	}
	else {
		if (atOrUnder(terrainHeight)) {
			jump(float3(0.0f, 1.0f, 0.0), 2.5f);
		}
	}

	if (withinDistanceTo(playerPosition, m_activationRadius)) {
		changeState(ACTIVE);
	}
}

void Apple::behaviorActive(float3 playerPosition) {
	ErrorLogger::log("Apple:: Doing active.");
	flee(playerPosition);
	if (!withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(PASSIVE);
	}
}

void Apple::behaviorCaught(float3 playerPosition) {
	ErrorLogger::log("Apple:: Doing caught.");
	m_directionalVelocity = playerPosition - m_position; // run to player
	m_directionalVelocity.Normalize();

	if (withinDistanceTo(playerPosition, 1.0f)) {
		// delete yourself
		ErrorLogger::log("Apple:: is picked up");
	}
}


void Apple::updateAnimated(float dt) {
	m_startAnimationPosition = m_position;
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes
	float3 posOrder[6] = {
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
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
			setAnimationDestination();
			justChanged = true;
			setRotation(float3(0.f, findRequiredRotation(m_nextDestinationAnimationPosition), 0.f));
		}

		m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
			frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)]);
	}
	if (m_currentFramePhase >= 2 || justChanged) {
		posOrder[0] = getPosition();
		posOrder[1] = getPosition();
	}
	// Set position
	/*float3 pos = XMVectorLerp(posOrder[m_currentFramePhase],
		posOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)], m_frameTime);
	setPosition(pos);*/

	// Update mesh specificly with our frametime
	m_meshAnim.updateSpecific(m_frameTime);
}


void Apple::flee(float3 playerPos) {
	/*float3 start = float3(m_startAnimationPosition.x, 0.0, m_startAnimationPosition.z);
	float3 end = float3(playerPos.x, 0.0f, playerPos.z);
	pathfinding(start, end);*/
	if (atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {
		m_directionalVelocity += m_position - playerPos;
		m_directionalVelocity.Normalize();
		m_directionalVelocity *= 3.f;
	}
}
