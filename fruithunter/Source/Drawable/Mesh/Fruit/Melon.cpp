#include "Melon.h"

Melon::Melon(float3 pos) : Fruit(pos) {
	loadAnimated("Melon", 1);
	m_nrOfFramePhases = 6;
	m_meshAnim.setFrameTargets(0, 0);
	m_rollSpeed = 5.f;
	m_fruitType = MELON;

	setCollisionData(getPosition(),
		m_meshAnim.getBoundingBoxHalfSizes().y); // sets to sphere with radius same as y halfsize;
	setScale(0.5);
	changeState(AI::State::PASSIVE);
	setStartPosition(pos);
	ErrorLogger::log("Melon:: going to second home");
	// enforce that homes are on terrain
	m_worldHome.y = TerrainManager::getInstance()->getHeightFromPosition(m_worldHome);
	m_secondWorldHome = m_worldHome + float3(3.f, 0.0, 3.0f);
	m_secondWorldHome.y = TerrainManager::getInstance()->getHeightFromPosition(m_secondWorldHome);
	m_directionalVelocity = m_position - m_secondWorldHome;
	m_directionalVelocity.Normalize();
	m_rollAnimationSpeed = 2.0f;
}

void Melon::behaviorPassive(float3 playerPosition) {
	if (withinDistanceTo(m_worldHome, 0.1f)) {
		ErrorLogger::log("Melon:: going to second home");
		m_directionalVelocity = m_secondWorldHome - m_position;
		lookTo(m_secondWorldHome);
		m_directionalVelocity.Normalize();
	}
	else if (withinDistanceTo(m_secondWorldHome, 0.1f)) {
		ErrorLogger::log("Melon:: going to first home");
		m_directionalVelocity = m_worldHome - m_position;
		lookTo(m_worldHome);
		m_directionalVelocity.Normalize();
	}

	if (withinDistanceTo(playerPosition, 4.0f)) {
		changeState(ACTIVE);
	}
}

void Melon::behaviorActive(float3 playerPosition) {
	ErrorLogger::log("Melon:: Doing active.");

	circulateAround(playerPosition);
	// pathfinding(m_position, sideStep - m_position);


	if (withinDistanceTo(playerPosition, 5.0f)) {
		changeState(PASSIVE);
	}
}

void Melon::behaviorCaught(float3 playerPosition) {
	ErrorLogger::log("Melon:: Doing caught.");
	m_directionalVelocity = playerPosition - m_position; // run to player
	m_directionalVelocity.Normalize();

	if (withinDistanceTo(playerPosition, 1.0f)) {
		// delete yourself
		ErrorLogger::log("Melon:: is picked up");
	}
}

void Melon::roll(float dt) { rotateX(dt * m_rollAnimationSpeed); }

void Melon::move(float dt) {
	if (!m_availablePath.empty() && m_currentState == ACTIVE) {
		m_directionalVelocity = (m_availablePath.back() - m_position);
		m_availablePath.pop_back();
	}
	m_directionalVelocity += m_acceleration * dt * dt / 2.0f;
	m_position += m_directionalVelocity * dt;
	enforceOverTerrain();
	setPosition(m_position);
}

void Melon::circulateAround(float3 playerPosition) {
	float3 toPlayer = m_position - playerPosition;
	float3 sideStep = toPlayer.Cross(float3(0.0f, 1.0f, 0.0f));
	if (toPlayer.Length() > 5.f) {
		sideStep -= toPlayer;
	}
	else if (toPlayer.Length() < 5.f) {

		sideStep += toPlayer;
	}
	m_directionalVelocity = sideStep;
}


void Melon::update(float dt, float3 playerPosition) {
	doBehavior(playerPosition);
	updateAnimated(dt);
	move(dt);
}
//
// void Melon::circulateAround(float3 playerPosition) {
//
//	float3 playerToMelon = m_position - playerPosition;
//	// playerToMelon.Transform();
//	float angle = 3.1416 * 0.25; // 8th of halfcircle
//
//	// rotate to find target
//	/*float3 rotated = (float4x4::CreateRotationY(angle).
//					  float4(playerToMelon.x, playerToMelon.y, playerToMelon.z, 1.0f))
//						 .xyz;*/
//	float3 target = playerToMelon;
//
//
//
//	ErrorLogger::log("inside circulateAround, target: (" + std::to_string(target.x) + " , " +
//					 std::to_string(target.y) + " , " + std::to_string(target.z) + ")");
//	pathfinding(m_position, playerPosition);
//}

void Melon::updateAnimated(float dt) {
	m_frameTime += dt;
	if (m_frameTime > 2) {
		m_frameTime = 0.f;
		setDestination();
		lookTo(m_destinationAnimationPosition);
	}
	float3 tempDir(m_destinationAnimationPosition - getPosition());
	tempDir.Normalize();
	roll(dt);
}

void Melon::setRollSpeed(float rollSpeed) { m_rollAnimationSpeed = rollSpeed; }
