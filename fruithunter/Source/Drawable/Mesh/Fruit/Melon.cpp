#include "Melon.h"

Melon::Melon(float3 pos) : Fruit(pos) {
	loadAnimated("Melon", 1);
	m_nrOfFramePhases = 6;
	m_meshAnim.setFrameTargets(0, 0);
	m_rollSpeed = 5.f;
	m_fruitType = MELON;

	setScale(0.5);
	changeState(AI::State::PASSIVE);
	setStartPosition(pos);
	// enforce that homes are on terrain
	setWorldHome(m_position);
	m_secondWorldHome = m_worldHome + float3(3.f, 0.0, 3.0f);
	m_secondWorldHome.y = TerrainManager::getInstance()->getHeightFromPosition(m_secondWorldHome);
	m_directionalVelocity = m_position - m_secondWorldHome;
	m_directionalVelocity.Normalize();
	m_rollAnimationSpeed = 2.0f;
	setCollisionDataOBB();
}

void Melon::behaviorPassive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	if (withinDistanceTo(m_worldHome, 0.75f)) {
		m_directionalVelocity = m_secondWorldHome - m_position;
		lookTo(m_secondWorldHome);
		m_directionalVelocity.Normalize();
	}
	else if (withinDistanceTo(m_secondWorldHome, 0.75f)) {
		m_directionalVelocity = m_worldHome - m_position;
		lookTo(m_worldHome);
		m_directionalVelocity.Normalize();
	}
	else if (!withinDistanceTo(m_worldHome, 5.f) && !withinDistanceTo(m_secondWorldHome, 5.f)) {
		m_directionalVelocity = m_worldHome - m_position;
		lookTo(m_worldHome);
		m_directionalVelocity.Normalize();
	}

	if (withinDistanceTo(playerPosition, 4.0f)) {
		changeState(ACTIVE);
	}
}

void Melon::behaviorActive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {

	circulateAround(playerPosition);
	// pathfinding(m_position, sideStep - m_position);

	if (!withinDistanceTo(playerPosition, 5.0f)) {
		changeState(PASSIVE);
	}
}

void Melon::behaviorCaught(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	m_directionalVelocity = playerPosition - m_position; // run to player
	m_directionalVelocity.Normalize();

	if (withinDistanceTo(playerPosition, 1.0f)) {
		// delete yourself
	}
}

void Melon::roll(float dt) { rotateX(dt * m_rollAnimationSpeed); }

void Melon::circulateAround(float3 playerPosition) {
	float3 toPlayer = m_position - playerPosition;
	float3 sideStep = toPlayer.Cross(float3(0.0f, 1.0f, 0.0f));
	if (toPlayer.Length() > 5.f) {
		sideStep -= toPlayer;
	}
	/*else if (toPlayer.Length() < 5.f) {

		sideStep += toPlayer;
	}*/
	m_directionalVelocity = sideStep;
}


void Melon::updateAnimated(float dt) {
	m_frameTime += dt;
	if (m_frameTime > 2) {
		m_frameTime = 0.f;
		setAnimationDestination();
		lookTo(m_destinationAnimationPosition);
	}
	float3 tempDir(m_destinationAnimationPosition - getPosition());
	tempDir.Normalize();
	roll(dt);
}

void Melon::setRollSpeed(float rollSpeed) { m_rollAnimationSpeed = rollSpeed; }
