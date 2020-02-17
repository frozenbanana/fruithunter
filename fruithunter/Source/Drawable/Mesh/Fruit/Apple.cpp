#include "Apple.h"
#include "Input.h"

Apple::Apple(float3 pos) : Fruit(pos) {
	loadAnimated("Bouncing_apple", 3);
	m_nrOfFramePhases = 6;
	setScale(0.5);
	changeState(AI::State::PASSIVE);

	m_directionalVelocity = float3((float)(rand() % 1), 0.0f, (float)(rand() % 1));
	m_directionalVelocity.Normalize();
	m_fruitType = APPLE;
}

void Apple::behaviorPassive(float3 playerPosition) {
	ErrorLogger::log("Apple:: Doing passive.");
	m_directionalVelocity = m_worldHome - m_position;
	m_directionalVelocity.Normalize();
	if (onGround(0.2f) && withinDistanceTo(m_worldHome, 2.0f)) {
		float3 terrainNormal = TerrainManager::getInstance()->getNormalFromPosition(m_position);
		terrainNormal.Normalize();
		jump(terrainNormal, 5.0);
	}

	if (withinDistanceTo(playerPosition, 3.f)) {
		changeState(ACTIVE);
	}
}

void Apple::behaviorActive(float3 playerPosition) {
	ErrorLogger::log("Apple:: Doing active.");
	m_directionalVelocity = m_position - playerPosition; // run away from player
	m_directionalVelocity.Normalize();
	if (!withinDistanceTo(playerPosition, 4.0f)) {
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
			setDestination();
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

// void Apple::move(float dt) {
//	// if (!m_availablePath.empty() && m_currentState == ACTIVE) {
//	//	m_direction = (m_availablePath.back() - m_position);
//	//	m_availablePath.pop_back();
//	//}
//
//	m_directionalVelocity += m_acceleration * dt * dt / 2.f;
//	m_position += m_directionalVelocity * dt;
//	// TODO: check if legal
//	// CURRENT: Enforece terrain height
//	enforceOverTerrain();
//	setPosition(m_position);
//}
//
// void Apple::update(float dt, float3 playerPosition) {
//	doBehavior(playerPosition);
//	updateAnimated(dt);
//	move(dt);
//}

void Apple::flee(float3 playerPos) {
	float3 start = float3(m_startAnimationPosition.x, 0.0, m_startAnimationPosition.z);
	float3 end = float3(playerPos.x, 0.0f, playerPos.z);
	pathfinding(start, end);
}
