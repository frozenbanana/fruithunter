#include "DragonFruit.h"
#include "PathFindingThread.h"
#include "TerrainManager.h"

bool DragonFruit::isFalling() { return m_velocity.y < 0.f && !m_onGround; }

DragonFruit::DragonFruit(float3 pos) : Fruit(pos) {
	loadAnimated("Dragon", 3);
	vector<string> names{ "Dragon.mtl", "Dragon2bronze.mtl", "Dragon2silver.mtl", "Dragon2gold.mtl",
		"Dragon3.mtl" };
	loadMaterials(names);

	m_nrOfFramePhases = 2;

	changeState(AI::State::PASSIVE);

	m_fruitType = DRAGON;

	// TEMP TAKEN FROM APPLE
	m_activeRadius = 30.f;
	m_passiveRadius = 32.f;

	m_passive_speed = 12.f;
	m_active_speed = 23.f;
	m_caught_speed = 18.f;

	m_wingStrength = 10.f;

	setCollisionDataOBB();
	m_groundFriction = 10.f;
	m_airFriction = m_groundFriction;
}


void DragonFruit::pathfinding(float3 start, std::vector<float4>* animals) {}

float3 DragonFruit::getRandomTarget() {
	float3 targetXZ = float3(RandomFloat(100.f, 200.f), 0.f, RandomFloat(100.f, 200.f));
	float height = TerrainManager::getInstance()->getHeightFromPosition(targetXZ);
	height = max(20.f, height);
	return float3(targetXZ.x, RandomFloat(height, height + 25.f), targetXZ.z);
}

void DragonFruit::behaviorPassive(float3 playerPosition) {
	// Perch on ground or on tree?
	

	if (withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(ACTIVE);

		jump(float3(0.0f, 1.f, 0.f), 5.f);
		m_target = getRandomTarget();
		return;
	}
	if (withinDistanceTo(m_target, 1.0f)) {
		// set new target
		m_target = getRandomTarget();
	}
	float terrainHeight = TerrainManager::getInstance()->getHeightFromPosition(m_position);
	float3 terrainPos = float3(m_position.x, terrainHeight, m_position.z);
	if (withinDistanceTo(terrainPos, 1.0f)) {
		jump(float3(0.0f, 1.0f, 0.0f), 15.f);
		m_target = getRandomTarget();
	}
	else if (withinDistanceTo(terrainPos, 1.0f)) {
		m_target = getRandomTarget();
	}


	m_direction = float3(m_target - m_position);
	lookToDir(m_velocity);
	m_speed = m_passive_speed;
}

void DragonFruit::behaviorActive(float3 playerPosition) {
	// when player is near, take flight
	if (!withinDistanceTo(playerPosition, m_activeRadius)) {
		changeState(PASSIVE);
		return;
	}
	m_target = m_position + (m_position - playerPosition);
	m_velocity = float3(m_target - m_position);
	m_velocity.Normalize();
	lookToDir(m_velocity);
	m_speed = m_active_speed;
}

void DragonFruit::behaviorCaught(float3 playerPosition) {
	// just go to player
	m_direction = playerPosition - m_position;
	lookToDir(m_direction);
	m_gravity = float3(0.f);
	m_speed = m_caught_speed;
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