#include "DragonFruit.h"
#include "PathFindingThread.h"
#include "SceneManager.h"

bool DragonFruit::isFalling() { return m_velocity.y < 0.f && !m_onGround; }

DragonFruit::DragonFruit(float3 pos) : Fruit(FruitType::DRAGON, pos) {
	// loadAnimated("Dragon", 3);
	loadAnimated("dragonfruit", 4);
	m_nrOfFramePhases = 2;

	changeState(AI::State::PASSIVE);

	// TEMP TAKEN FROM APPLE
	m_activeRadius = 30.f;
	m_passiveRadius = 32.f;

	m_passive_speed = 12.f;
	m_active_speed = 23.f;
	m_caught_speed = 18.f;

	m_wingStrength = 10.f;

	setCollisionDataOBB();
	m_groundFriction = 60.f;
	m_airFriction = 60;

	m_gravity = float3(0.);
	m_direction = float3(0.);
}


void DragonFruit::pathfinding(float3 start, std::vector<float4>* animals) {}

float3 DragonFruit::getRandomTarget() {
	Terrain* terrain = SceneManager::getScene()->m_terrains.getTerrainFromPosition(m_worldHome);
	float3 t_pos = terrain->getPosition();
	float3 t_size = terrain->getScale();
	float spawn_mid = (m_spawn_range.y + m_spawn_range.x) / 2.f;
	float randomHeight = 0;
	if (m_worldHome.y > t_size.y + spawn_mid) {
		randomHeight = RandomFloat(m_spawn_range.x, spawn_mid);
	}
	else {
		randomHeight = RandomFloat(spawn_mid, m_spawn_range.y);
	}
	return float3(RandomFloat(0, t_size.x), t_size.y + randomHeight, RandomFloat(0, t_size.z)) +
		   t_pos;

	// float3 targetXZ = float3(RandomFloat(100.f, 200.f), 0.f, RandomFloat(100.f, 200.f));
	// float height = SceneManager::getScene()->m_terrains.getHeightFromPosition(targetXZ);
	// height = max(20.f, height);
	// return float3(targetXZ.x, RandomFloat(height, height + 25.f), targetXZ.z);
}

void DragonFruit::_onDeath(Skillshot skillshot) { spawnCollectionPoint(skillshot); }

void DragonFruit::behaviorPassive() {
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	Terrain* terrain = SceneManager::getScene()->m_terrains.getTerrainFromPosition(m_worldHome);
	float3 t_size = terrain->getScale();

	float3 position = getPosition();
	position.y = t_size.y + m_spawn_range.y;
	setPosition(position);
	changeState(ACTIVE);

	// Perch on ground or on tree?

	// if (withinDistanceTo(playerPosition, m_passiveRadius)) {
	//	changeState(ACTIVE);

	//	jump(float3(0.0f, 1.f, 0.f), 5.f);
	//	m_target = getRandomTarget();
	//	return;
	//}
	// if (withinDistanceTo(m_target, 1.0f)) {
	//	// set new target
	//	m_target = getRandomTarget();
	//}
	// float terrainHeight =
	// SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition()); float3 terrainPos
	// = float3(getPosition().x, terrainHeight, getPosition().z); if
	// (withinDistanceTo(terrainPos, 1.0f)) { 	jump(float3(0.0f, 1.0f, 0.0f), 15.f); 	m_target =
	// getRandomTarget();
	//}
	// else if (withinDistanceTo(terrainPos, 1.0f)) {
	//	m_target = getRandomTarget();
	//}


	// m_direction = float3(m_target - getPosition());
	// lookTo(m_velocity * float3(1, 0, 1));
	// m_speed = m_passive_speed;
}

void DragonFruit::behaviorActive() {
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	float dt = SceneManager::getScene()->getDeltaTime();

	// timer for retargeting
	m_timer -= dt;
	if (m_timer <= 0) {
		m_timer = RandomFloat(m_timer_range.x, m_timer_range.y);
		m_worldHome = getRandomTarget();
	}

	// speed gain from forward tilt
	float forwardTilt = Normalize(m_velocity).Dot(float3(0, -1, 0));
	m_velocitySpeed += forwardTilt * m_speedGain * dt;

	// animation based on tilt
	m_animationSpeed = m_animationSpeed_base - forwardTilt * m_animationSpeed_changeOnTilt;

	// look in direction it is moving
	lookTo(Normalize(m_velocity));

	// velocity direction change
	float heightFromGround =
		getPosition().y - SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition());
	float3 avoidFloor_force = float3(0, 1, 0) * (m_heightFromGroundLimit - heightFromGround) *
							  (1 - Clamp<float>(heightFromGround / m_heightFromGroundLimit, 0, 1));

	float3 oldVelocity = m_velocity;
	float3 target_force = (Normalize(m_worldHome - getPosition()) - Normalize(m_velocity)) *
						  m_steeringSpeed * m_velocitySpeed;
	// m_velocity += (target_force)*dt + avoidFloor_force*dt*60;
	m_velocity += (target_force)*dt + avoidFloor_force * dt * 15;
	m_velocity = Normalize(m_velocity) * m_velocitySpeed;

	// z rotation, sideway tilt when turning
	float2 oldVel2D = float2(oldVelocity.x, oldVelocity.z);
	oldVel2D.Normalize();
	float2 oldVel2DTurned90 = rotatef2(oldVel2D, 3.1415f / 2.0f);
	float2 vel2D = float2(m_velocity.x, m_velocity.z);
	vel2D.Normalize();
	float desired_rotZ = acos(Clamp<float>(oldVel2D.Dot(vel2D), -1, 1)) *
						 (oldVel2DTurned90.Dot(vel2D) > 0 ? 1 : -1) * m_turningStrength;
	m_rotZ = m_rotZ + (desired_rotZ - m_rotZ) * dt * 3;
	float3 rotation = getRotation();
	rotation.z = m_rotZ;
	setRotation(rotation);

	// when player is near, take flight
	// if (!withinDistanceTo(playerPosition, m_activeRadius)) {
	//	changeState(PASSIVE);
	//	return;
	//}
	// m_target = getPosition() + (getPosition() - playerPosition);
	// m_velocity = float3(m_target - getPosition());
	// m_velocity.Normalize();
	// lookTo(m_velocity * float3(1, 0, 1));
	// m_speed = m_active_speed;
}

void DragonFruit::behaviorCaught() {
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	// just go to player
	m_direction = playerPosition - getPosition();
	lookTo(m_direction * float3(1, 0, 1));
	m_gravity = float3(0.f);
	m_speed = m_caught_speed;
	if (!m_ascend) {
		jump(float3(0.f, 1.f, 0.f), 20.f);
		m_ascend = true;
	}
}


void DragonFruit::updateAnimated(float dt) {
	m_frameTime += dt * m_animationSpeed;
	m_frameTime = fmodf(m_frameTime, 4.f);

	// if (m_frameTime < 1)
	//	setFrameTargets(0, 1);
	// else if (m_frameTime < 2)
	//	setFrameTargets(1, 2);
	// else if (m_frameTime < 3)
	//	setFrameTargets(2, 1);
	// else if (m_frameTime < 4)
	//	setFrameTargets(1, 0);

	if (m_frameTime < 1)
		setFrameTargets(0, 1);
	else if (m_frameTime < 2)
		setFrameTargets(1, 2);
	else if (m_frameTime < 3)
		setFrameTargets(2, 3);
	else if (m_frameTime < 4)
		setFrameTargets(3, 0);

	m_meshAnim.updateSpecific(m_frameTime);
}