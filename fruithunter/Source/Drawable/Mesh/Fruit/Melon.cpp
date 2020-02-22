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

	if (withinDistanceTo(playerPosition, m_activeRadius)) {
		changeState(ACTIVE);
	}
}

void Melon::behaviorActive(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {

	float3 target =  circulateAround(playerPosition);
	if (m_availablePath.empty()) {
		pathfinding(m_position, target, collidables);

	}
	if (!m_availablePath.empty())
		m_directionalVelocity = (m_availablePath.back() - m_position)*2.f;

	if (!withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(PASSIVE);
	}
}

void Melon::behaviorCaught(float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	m_directionalVelocity = playerPosition - m_position; // run to player
	m_directionalVelocity.Normalize();

}

void Melon::roll(float dt) { rotateX(dt * m_rollAnimationSpeed); }

float3 Melon::circulateAround(float3 playerPosition) {

	float3 toMelon = m_position - playerPosition;
	toMelon.y = playerPosition.y;
	float angle = XM_PI / 8;
	Matrix rotate = Matrix(
		cos(angle), 0.f, -sin(angle), 0.f,
		0.f, 1.f, 0.f, 0.f,
		sin(angle), 0.f, cos(angle), 0.f, 
		0.f, 0.f, 0.f, 1.f);


	float3 target = target.Transform(toMelon, rotate);
	target.Normalize();
	target *= 10.f;

	target += playerPosition;
	target.y = playerPosition.y;

	/*float3 toPlayer = m_position - playerPosition;
	float3 sideStep = toPlayer.Cross(float3(0.0f, 1.0f, 0.0f));
	if (toPlayer.Length() > 5.f) {
		sideStep -= toPlayer;
	}
	*/
	return target;
	//m_directionalVelocity = (target - m_position)*2.f;

	/*rotate = Matrix(cos(-angle), 0.f, -sin(-angle), 0.f, 0.f, 1.f, 0.f, 0.f, sin(-angle), 0.f,
		cos(-angle), 0.f, 0.f, 0.f, 0.f, 1.f);
	m_directionalVelocity.Transform(m_directionalVelocity, rotate);*/
	
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
