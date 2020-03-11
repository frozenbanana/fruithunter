#include "DragonFruit.h"

DragonFruit::DragonFruit(float3 pos) {}

void DragonFruit::waveFlight(float3 playerPosition, float radius) {


	int degrees = 10 % 360;
	float radians = degrees * XM_PI / 180.f;
	float flightPattern = sin(radians);
}

void DragonFruit::circulateVertical(float3 playerPosition, float radius) {
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
}

void DragonFruit::circulate(float3 playerDir, float radius) {}

void DragonFruit::behaviorPassive(float3 playerPosition) {
	// Perch on ground or on tree?
	if (withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(ACTIVE);
		return;
	}
	if (m_onGround) {
		circulate(playerPosition, 0.f);
	}
	else {
		m_direction = m_worldHome - m_position;
		if (m_velocity.y < 0.f && m_position.y < m_flapWings) {
			float3 target = m_direction;
			target.Normalize();
			target.y = 1.f;
			jump(target, m_wingStrength);
		}
	}

	m_speed = m_passive_speed;
}

void DragonFruit::behaviorActive(float3 playerPosition) {
	// when player is near, take flight

	// circulate player in air.
}

void DragonFruit::behaviorCaught(float3 playerPosition) {
	// just go to player
}


DragonFruit::DragonFruit() {
	loadAnimated("Dragon", 3);
	vector<string> names{ "Dragon1.mtl", "Dragon2.mtl", "Dragon3.mtl" };
	loadMaterials(names, 3);

	m_nrOfFramePhases = 2;

	changeState(AI::State::PASSIVE);

	m_fruitType = DRAGON;

	// TEMP TAKEN FROM APPLE
	m_activeRadius = 8.f;
	m_passiveRadius = 12.f;

	m_passive_speed = 3.f;
	m_active_speed = 10.f;
	m_caught_speed = 5.f;

	setCollisionDataOBB();
}

DragonFruit::~DragonFruit() {}

void DragonFruit::updateAnimated(float dt) {
	m_frameTime += dt * 4;
	m_frameTime = fmodf(m_frameTime, 4.f);

	if (m_frameTime < 1)
		setFrameTargets(0, 1);
	else if (m_frameTime < 2)
		setFrameTargets(1, 2);
	else if (m_frameTime < 3)
		setFrameTargets(2, 1);
	else if (m_frameTime < 4)
		setFrameTargets(1, 0);
	m_meshAnim.updateSpecific(m_frameTime);

	return;
}
