#include "Melon.h"

Melon::Melon(float3 pos) : Fruit(pos) {
	loadAnimated("Melon", 1);
	m_nrOfFramePhases = 6;
	vector<string> names{ "Melon.mtl", "Melon2bronze.mtl", "Melon2silver.mtl", "Melon2gold.mtl",
		"Melon3.mtl" };
	loadMaterials(names);

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
	m_direction = m_position - m_secondWorldHome;

	m_rollAnimationSpeed = 2.0f;
	setCollisionDataOBB();


	m_passiveRadius = 15.f;
	m_activeRadius = 15.f;

	m_passive_speed = 8.f;
	m_active_speed = 15.f;
	m_caught_speed = 10.f;
}

void Melon::behaviorPassive(float3 playerPosition) {

	if (m_position.y <= 1.f) {
		float3 target = m_worldHome - m_position;
		target.Normalize();
		target.y = 1.f;
		jump(target, 10.f);
		return;
	}


	if (m_onGround) {

		if (withinDistanceTo(m_worldHome, 0.75f)) {
			m_direction = m_secondWorldHome - m_position;
			lookTo(m_secondWorldHome);
		}
		else if (withinDistanceTo(m_secondWorldHome, 0.75f)) {
			m_direction = m_worldHome - m_position;
			lookTo(m_worldHome);
		}
		else if (!withinDistanceTo(m_worldHome, 5.f) && !withinDistanceTo(m_secondWorldHome, 5.f)) {
			m_direction = m_worldHome - m_position;
			lookTo(m_worldHome);
		}
		m_speed = m_passive_speed;
		if (withinDistanceTo(playerPosition, m_activeRadius)) {
			changeState(ACTIVE);
		}
	}
}

void Melon::behaviorActive(float3 playerPosition) {
	if (m_onGround) {

		if (m_availablePath.empty()) {
			float3 target = circulateAround(playerPosition);
			makeReadyForPath(target);
		}

		lookTo(playerPosition);
		m_speed = m_active_speed;

		if (!withinDistanceTo(playerPosition, m_passiveRadius)) {
			stopMovement();
			changeState(PASSIVE);
		}
	}
}

void Melon::behaviorCaught(float3 playerPosition) {
	if (atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {
		m_direction = playerPosition - m_position; // run to player

		m_speed = m_caught_speed;
		makeReadyForPath(playerPosition);
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
