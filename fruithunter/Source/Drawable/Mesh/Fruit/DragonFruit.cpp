#include "DragonFruit.h"
#include "PathFindingThread.h"

bool DragonFruit::isFalling() { return m_velocity.y < 0.f && !m_onGround; }

DragonFruit::DragonFruit(float3 pos) : Fruit(pos) {
	loadAnimated("Dragon", 3);
	vector<string> names{ "Dragon.mtl", "Dragon2.mtl", "Dragon3.mtl", "Dragon2.mtl",
		"Dragon3.mtl" };
	loadMaterials(names, 5);

	m_nrOfFramePhases = 2;

	changeState(AI::State::PASSIVE);

	m_fruitType = DRAGON;

	// TEMP TAKEN FROM APPLE
	m_activeRadius = 30.f;
	m_passiveRadius = 32.f;

	m_passive_speed = 3.f;
	m_active_speed = 10.f;
	m_caught_speed = 5.f;

	m_wingStrength = 10.f;

	setCollisionDataOBB();
	m_airFriction = m_groundFriction;
}



void DragonFruit::waveFlight(float3 playerPosition) {

	if (isFalling()) {
		PathFindingThread* pft = PathFindingThread::getInstance();
		float3 target;
		if (m_availablePath.size() > 0) {
			target = m_availablePath.back();
			// jump(target, 5.f);
		}
		else {
			target = m_direction;
			target.Normalize();
			target.y = 1.f;
			target += m_direction;
			makeReadyForPath(target);
			// jump(target, 5.f);
		}
	}
}

void DragonFruit::circulateVertical(float3 playerPosition, float radius) {


	float3 levitationPoint = m_position - playerPosition;
	levitationPoint.y = playerPosition.y;



	float angle = XM_PI / 16;
	Matrix rotate = Matrix(cos(angle), 0.f, -sin(angle), 0.f, 0.f, 1.f, 0.f, 0.f, sin(angle), 0.f,
		cos(angle), 0.f, 0.f, 0.f, 0.f, 1.f);


	float3 target = target.Transform(levitationPoint, rotate);
	target.Normalize();
	target *= radius;

	target += playerPosition;

	float groundPoint =
		max(playerPosition.y, TerrainManager::getInstance()->getHeightFromPosition(target));
	target.y = groundPoint + 15.f;
	// target *= 10.f;
	m_direction = target - m_position;
	lookToDir(playerPosition - m_position);
}

void DragonFruit::setDirection() {

	if (!m_availablePath.empty()) {
		m_direction = m_availablePath.back() - m_position;
		m_direction.Normalize();
		m_direction.y = 1.f;
		jump(m_direction, m_wingStrength);
	}
	updated = false;
}

void DragonFruit::pathfinding(float3 start,  std::vector<float4> *animals) {
	
}


void DragonFruit::behaviorPassive(float3 playerPosition) {
	// Perch on ground or on tree?


	if (withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(ACTIVE);
		
		jump(float3(0.0f, 1.f, 0.f), 40.f);
		return;
	}
	if (!withinDistanceTo(m_worldHome + float3(0.f, 6.f, 0.f), 0.f)) {
		makeReadyForPath(m_worldHome);
	}
	else if (!m_onGround) {
		m_direction = float3(0.f, -1.f, 0.f);
	}
	else {
		lookToDir(m_direction);
	}

	m_speed = m_passive_speed;
}

void DragonFruit::behaviorActive(float3 playerPosition) {
	// when player is near, take flight
	if (!withinDistanceTo(playerPosition, m_activeRadius)) {
		changeState(PASSIVE);
		
		return;
	}
	// circulate player in air.
	// m_gravity = float3(0.f);
	auto height = TerrainManager::getInstance()->getHeightFromPosition(m_position);
	
	if (min(abs(m_position.y - playerPosition.y), abs(m_position.y - height)) > 5.f &&
		isFalling()) {
		jump(float3(0.f, 1.f, 0.f), m_wingStrength);
	}

	if (min(abs(m_position.y - playerPosition.y), abs(m_position.y - height) )> 5.f) {
		circulateVertical(playerPosition, 17.f);
		waveFlight(playerPosition);
	}
	else {
		circulateVertical(playerPosition, 15.f);
		//jump(m_direction, 1.f);
	}

	m_speed = 20.f;
	m_wingStrength = 0.f;
}

void DragonFruit::behaviorCaught(float3 playerPosition) {
	// just go to player

	m_direction = playerPosition - m_position;
	lookToDir(m_direction);
	m_gravity = float3(0.f);
	m_speed = 10.f;
	if (!m_ascend) {
		jump(float3(0.f, 1.f, 0.f), 20.f);
		m_ascend = true;
	}
}


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