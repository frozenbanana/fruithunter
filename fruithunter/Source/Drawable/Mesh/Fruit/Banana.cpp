#include "Banana.h"


Banana::Banana(float3 pos) : Fruit(pos) {
	m_fruitType = BANANA;
	loadAnimated("Banana", 3);
	vector<string> names{ "Banana.mtl", "Banana2.mtl", "Banana3.mtl" };
	loadMaterials(names, 3);
	m_nrOfFramePhases = 5;
	m_maxBounciness = 3;
	m_bounciness = 0;
	m_state = Jump;
	rotRandom();
	setScale(2.f);
	m_currentState = PASSIVE;
	m_worldHome = m_position;
	setCollisionDataOBB();

	m_activeRadius = 5.f;
	m_passiveRadius = 0.f;
	m_speed = 1.f;

	setFrameTargets(0, 1);
}

void Banana::behaviorPassive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	TerrainManager* terrainManger = TerrainManager::getInstance();
	float terrainHeight = terrainManger->getHeightFromPosition(m_position);
	// Only decide what to do on ground
	if (atOrUnder(terrainHeight)) {
		float3 direction = float3(0.f);
		if (!withinDistanceTo(m_worldHome, ARRIVAL_RADIUS)) {
			float3 toHome = m_worldHome - m_position;
			toHome.Normalize();
			toHome.y = 1.0f;
			direction = toHome;
		}
		else {
			float3 terrainNormal = terrainManger->getNormalFromPosition(m_position);
			terrainNormal.Normalize();
			direction = terrainNormal;
		}
		jump(direction, 4.0f);

		if (withinDistanceTo(playerPosition, m_activeRadius)) {
			changeState(ACTIVE);
		}
	}
}

void Banana::behaviorActive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	TerrainManager* terrainManger = TerrainManager::getInstance();
	float terrainHeight = terrainManger->getHeightFromPosition(m_position);
	// Only decide what to do on ground
	if (atOrUnder(terrainHeight)) {
		float3 terrainNormal = terrainManger->getNormalFromPosition(m_position);
		// Go bananas!
		terrainNormal.x += 0.1f * (float)(rand() % 1) - 0.5f;
		terrainNormal.z += 0.1f * (float)(rand() % 1) - 0.5f;
		terrainNormal.y = 1.0f;
		jump(terrainNormal, 1.0f);
		m_speed = 5.f;

		if (!withinDistanceTo(playerPosition, m_passiveRadius)) {
			changeState(PASSIVE);
		}
	}
}
void Banana::behaviorCaught(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {

	TerrainManager* terrainManger = TerrainManager::getInstance();
	float terrainHeight = terrainManger->getHeightFromPosition(m_position);

	if (atOrUnder(terrainHeight)) {
		float3 toPlayer = playerPosition - m_position;
		toPlayer.Normalize();
		toPlayer.y = 1.0f;
		m_speed = 1.0f;
		jump(toPlayer, 1.0f);
	}
}

void Banana::updateAnimated(float dt) {
	switch (m_state) {
	case Jump:
		updateFirstJump(dt);
		break;
	case Bounce:
		updateBounce(dt);
		break;
	case Stopped:
		updateStopped(dt);
		break;
	default:
		ErrorLogger::log("wrong state in banana");
	}
}

void Banana::release(float3 direction) {
	// start bouncing
	m_nrOfFramePhases = 3;
	m_bounciness = m_maxBounciness;
	m_state = Bounce;
	changeState(RELEASED);
	m_directionalVelocity = direction;
	m_directionalVelocity.Normalize();
	m_directionalVelocity *= 15.0f;
	afterRealease = true;
}


void Banana::updateFirstJump(float dt) {
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
	// Maybe change keyframes
	if (m_frameTime > 1) {
		m_frameTime -= 1;
		m_currentFramePhase = m_currentFramePhase + 1;

		if (m_currentFramePhase == m_nrOfFramePhases) {
			m_currentFramePhase = 0;
			justChanged = true;
			setRotation(float3(0.f, findRequiredRotation(m_nextDestinationAnimationPosition), 0.f));

			// start bouncing
			m_nrOfFramePhases = 3;
			m_bounciness = m_maxBounciness;
			m_state = Bounce;
			bounce();
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

void Banana::updateBounce(float dt) {
	// TODO: Bounce. physics?

	int frameOrder[] = { 1, 2, 1 }; // Order of using keyframes
	float3 posOrder[] = {
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
	};
	bool justChanged = false;
	float frameSpeedOrder[] = { 2.f, 2.0f, 15.9f };
	m_frameTime += dt * frameSpeedOrder[m_currentFramePhase];
	// Maybe change keyframes
	if (m_frameTime > 1) {
		m_frameTime -= 1;
		m_currentFramePhase = m_currentFramePhase + 1;

		if (m_currentFramePhase == m_nrOfFramePhases) {
			m_currentFramePhase = 0;
			justChanged = true;
			bounce();
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
	rotate(m_rotation * dt);
	// Update mesh specificly with our frametime
	m_meshAnim.updateSpecific(m_frameTime);
}

void Banana::updateStopped(float dt) {
	// TODO: Straighten up and prepare to jump again.
}

void Banana::stop() {}

void Banana::bounce() {
	setAnimationDestination();
	if (m_bounciness <= 0) { // Will be stopped
		m_state = Jump;
		m_nrOfFramePhases = 6;
	}
	else {
		m_heightAnimationPosition.y += m_bounciness;
		m_bounciness -= 0.3f;
		rotRandom();
	}
}
