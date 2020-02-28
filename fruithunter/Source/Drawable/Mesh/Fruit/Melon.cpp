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


	m_passiveRadius = 15.f;
	m_activeRadius = 15.f;
}

void Melon::behaviorPassive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	if (atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {

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
	}
	m_speed = 2.f;
	if (withinDistanceTo(playerPosition, m_activeRadius)) {
		changeState(ACTIVE);
	}
}

void Melon::behaviorActive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	if (atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {

		/*if (m_availablePath.empty()) {
			float3 target = circulateAround(playerPosition);
			pathfinding(m_position, target, collidables);
		}*/
	}

	lookTo(playerPosition);
	m_speed = 5.f;

	if (!withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(PASSIVE);
	}
}

void Melon::behaviorCaught(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	if (atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {
		m_directionalVelocity = playerPosition - m_position; // run to player
		m_directionalVelocity.Normalize();
		m_speed = 5.f;
	}
	lookTo(playerPosition);
}

void Melon::roll(float dt) { rotateX(dt * m_rollAnimationSpeed); }

float3 Melon::circulateAround(float3 playerPosition) {

	float3 toMelon = m_position - playerPosition;
	toMelon.y = playerPosition.y;
	float angle = XM_PI / 8;
	Matrix rotate = Matrix(cos(angle), 0.f, -sin(angle), 0.f, 0.f, 1.f, 0.f, 0.f, sin(angle), 0.f,
		cos(angle), 0.f, 0.f, 0.f, 0.f, 1.f);


	float3 target = target.Transform(toMelon, rotate);
	target.Normalize();
	target *= 10.f;

	target += playerPosition;
	target.y = playerPosition.y;

	return target;
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
