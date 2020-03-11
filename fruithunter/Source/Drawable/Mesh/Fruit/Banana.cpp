#include "Banana.h"
#include "Renderer.h"
#include "ErrorLogger.h"

#define PASSIVE_JUMP_POWER 15.f
#define ACTIVE_JUMP_POWER 35.f
#define PANIC_JUMP_POWER 40.f



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
	m_speed = 1.f;
	m_activeRadius = 5.f;
	m_passiveRadius = 3.f;

	setFrameTargets(0, 1);
}

void Banana::behaviorPassive(float3 playerPosition) {
	TerrainManager* terrainManger = TerrainManager::getInstance();

	if (m_position.y <= 1.f) {
		float3 target = m_worldHome - m_position;
		target.Normalize();
		target.y = 1.f;
		jump(target, 10.f);
		return;
	}
	// Only decide what to do on ground
	if (m_onGround) {
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
		jump(direction, PASSIVE_JUMP_POWER);

		if (withinDistanceTo(playerPosition, m_activeRadius)) {
			changeState(ACTIVE);
			stopMovement();
		}
	}
}

void Banana::behaviorActive(float3 playerPosition) {

	if (!withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(PASSIVE);
	}
	TerrainManager* terrainManger = TerrainManager::getInstance();
	// Only decide what to do on ground
	if (m_onGround) {
		float3 terrainNormal = terrainManger->getNormalFromPosition(m_position);
		// Go bananas!
		terrainNormal.x += RandomFloat(-1.f, 1.f);
		terrainNormal.z += RandomFloat(-1.f, 1.f);
		terrainNormal.y = 1.0f;
		jump(terrainNormal, ACTIVE_JUMP_POWER);
	}
}
void Banana::behaviorCaught(float3 playerPosition) {
	if (m_onGround) {
		float3 toPlayer = playerPosition - m_position;
		toPlayer.Normalize();
		toPlayer.y = 1.0f;
		jump(toPlayer, 8.0f);
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
	m_direction = direction;
	m_velocity = m_direction * THROWVELOCITY;
	m_afterRealease = true;
}


void Banana::updateFirstJump(float dt) {
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes

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
			lookTo(m_nextDestinationAnimationPosition);

			// start bouncing
			m_nrOfFramePhases = 3;
			m_bounciness = m_maxBounciness;
			m_state = Bounce;
			bounce();
		}

		m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
			frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)]);
	}

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
			lookTo(m_nextDestinationAnimationPosition);
		}

		m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
			frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)]);
	}
	if (m_currentFramePhase >= 2 || justChanged) {
		posOrder[0] = getPosition();
		posOrder[1] = getPosition();
	}

	rotate(m_rotation * dt);

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
