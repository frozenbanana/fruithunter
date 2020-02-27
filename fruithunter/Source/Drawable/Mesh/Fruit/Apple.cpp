#include "Apple.h"
#include "Input.h"

Apple::Apple(float3 pos) : Fruit(pos) {
	loadAnimated("Bouncing_apple", 3);
	m_nrOfFramePhases = 6;
	setScale(0.5);
	changeState(AI::State::PASSIVE);

	m_directionalVelocity = float3(1.f, 0.f, 1.f);
	m_fruitType = APPLE;

	m_activeRadius = 4.f;
	m_passiveRadius = 8.f;
	setCollisionDataOBB();
}

void Apple::behaviorPassive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	float terrainHeight = TerrainManager::getInstance()->getHeightFromPosition(m_position);
	// Check if not at home
	if (withinDistanceTo(playerPosition, m_activeRadius)) {
		m_speed = 0.0f;
		changeState(ACTIVE);
	}
	else {
		if (!withinDistanceTo(m_worldHome, ARRIVAL_RADIUS) && atOrUnder(terrainHeight)) {
			// Check if there is no other path on going
			if (m_availablePath.empty() && !m_lookingForPath) {
				if (m_nrOfTriesGoHome++ < 10) {
					pathfinding(m_position, m_worldHome, collidables); // go home
				}
				else {
					setWorldHome(m_position + float3(0.001f, 0, 0.001f));
					m_nrOfTriesGoHome = 0;
				}
				m_directionalVelocity =
					m_worldHome - m_position; // ensures homegoing even if path returns nothing
				m_directionalVelocity.Normalize();
				m_speed = 2.0f;
			}
		}
		else { // Just jump when home
			if (atOrUnder(terrainHeight)) {

				jump(float3(0.0f, 1.0f, 0.0), 2.5f);
			}
		}
	}
}

void Apple::behaviorActive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	if (!withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(PASSIVE);
		m_speed = 0.f;
	}
	else {
		flee(playerPosition, collidables);
		m_speed = 10.f;
	}
}

void Apple::behaviorCaught(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	if (atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {
		m_directionalVelocity = playerPosition - m_position; // run to player
		m_directionalVelocity.Normalize();
		m_directionalVelocity *= 6.0f;
	}
	lookTo(playerPosition);
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

	// Update mesh specificly with our frametime
	m_meshAnim.updateSpecific(m_frameTime);
}


void Apple::flee(float3 playerPos, vector<shared_ptr<Entity>> collidables) {
	// Update fleeing path if ther is none
	if (m_availablePath.empty() && !m_lookingForPath) {
		float3 runTo = m_position - playerPos;
		runTo.Normalize();
		runTo *= m_passiveRadius;
		runTo += m_position;
		pathfinding(m_position, runTo, collidables);
	}
	// set new velocity from path
}
