#include "DragonFruit.h"
#include "SceneManager.h"

bool DragonFruit::isFalling() { return m_velocity.y < 0.f && !m_onGround; }

DragonFruit::DragonFruit(float3 pos) : Fruit(FruitType::DRAGON, pos) {
	loadAnimated("dragonfruit", 4);
	setCollisionDataOBB();

	Environment* environment = SceneManager::getScene()->m_terrains.getTerrainFromPosition(pos);
	pos.y = environment->getPosition().y + environment->getScale().y + m_spawn_range.y;
	setPosition(pos);

	m_groundFriction = 60.f;
	m_airFriction = 60;

	m_gravity = float3(0.);

	// float r = RandomFloat() * 2 * XM_PI;
	// m_velocity = float3(cos(r), 0, sin(r)) * 1;
}


void DragonFruit::pathfinding(float3 start, std::vector<float4>* animals) {}

float3 DragonFruit::getRandomTarget() {
	Terrain* terrain = m_boundTerrain;
	float3 t_pos = terrain->getPosition();
	float3 t_size = terrain->getScale();
	float spawn_mid = (m_spawn_range.y + m_spawn_range.x) / 2.f;
	float randomHeight = 0;
	if (m_target.y > t_size.y + spawn_mid) {
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

void DragonFruit::update() {
	Scene* scene = SceneManager::getScene();
	float dt = scene->getDeltaTime();

	checkOnGroundStatus(); // checks if on ground
	updateAnimated(dt);	   // animation stuff
	updateVelocity(dt);	   // update velocity (slowdown and apply accelration)
	behavior();
	updateRespawn();
	move(dt);			  // update position from velocity
	enforceOverTerrain(); // force fruit above ground

	if (getPosition().y < 1)
		respawn();
}

void DragonFruit::_onDeath(Skillshot skillshot) { spawnCollectionPoint(skillshot); }

void DragonFruit::behavior() {
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	float dt = SceneManager::getScene()->getDeltaTime();

	// timer for retargeting
	m_timer -= dt;
	if (m_timer <= 0) {
		m_timer = RandomFloat(m_timer_range.x, m_timer_range.y);
		m_target = getRandomTarget();
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
	float3 target_force = (Normalize(m_target - getPosition()) - Normalize(m_velocity)) *
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
}

void DragonFruit::updateAnimated(float dt) {
	int frameCount = m_meshAnim.getNrOfMeshes();

	m_frameTime += dt * m_animationSpeed / frameCount;
	m_frameTime = fmod(m_frameTime, 1.f);

	float x = m_frameTime;
	float y = ((1 - cos((x * 2 + floor(x * 2)) * XM_PI)) + floor(x * 2) * 2) * 0.25f;

	float anim = y * frameCount;
	int frameIndex = floor(anim);
	float rest = anim - frameIndex;
	setFrameTargets(frameIndex%frameCount, (frameIndex + 1) % frameCount);
	m_meshAnim.updateSpecific(rest);
}