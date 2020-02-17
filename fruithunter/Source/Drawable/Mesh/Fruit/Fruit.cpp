#include "Fruit.h"
#include "Input.h"


void Fruit::jump(float3 direction, float power) { m_directionalVelocity = power * direction; }

void Fruit::setStartPosition(float3 pos) {
	setPosition(pos);
	setWorldHome(pos);
	m_startAnimationPosition = pos;
	m_heightAnimationPosition = pos;
	m_destinationAnimationPosition = pos;
	m_nextDestinationAnimationPosition = pos;
}

void Fruit::setNextDestination(float3 nextDest) {
	m_nextDestinationAnimationPosition =
		nextDest + float3(0.0f, 0.3f, 0.0f); // offset to adjust origo so pos it onup of terrain
}

void Fruit::lookTo(float3 lookAt) { setRotation(float3(0.f, findRequiredRotation(lookAt), 0.f)); }

int Fruit::getFruitType() { return m_fruitType; }

float Fruit::findRequiredRotation(float3 lookAt) {
	float rot = 0.f;

	float dx = lookAt.x - getPosition().x;
	float dz = lookAt.z - getPosition().z;
	if (dx != 0) {
		rot = -atan(dz / dx);
	}
	else {
		rot = 0;
	}

	if (dx < 0) {
		rot = 3.1416f + rot;
	}
	return rot + 3.14f * 0.5f;
}

void Fruit::enforceOverTerrain() {
	if (!onGround(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {
		m_position.y = TerrainManager::getInstance()->getHeightFromPosition(m_position) +
					   getHalfSizesAnimated().y / 2.f;
	}
}

void Fruit::setDestination() {
	m_destinationAnimationPosition = m_nextDestinationAnimationPosition;
	m_startAnimationPosition = getPosition();
	m_heightAnimationPosition =
		XMVectorLerp(m_startAnimationPosition, m_destinationAnimationPosition, 0.5f);
	m_heightAnimationPosition.y += 1.f;
}
void Fruit::setWorldHome(float3 pos) {
	m_worldHome = pos;
	m_worldHome.y = 0.f;
}

bool Fruit::withinDistanceTo(float3 target, float treshhold) {
	return (m_position - target).Length() < treshhold;
}

void Fruit::update(float dt, float3 playerPosition) {
	doBehavior(playerPosition);
	updateAnimated(dt);
	move(dt);
}

void Fruit::move(float dt) {
	m_directionalVelocity += m_acceleration * dt * dt / 2.f;
	m_position += m_directionalVelocity * dt;
	// TODO: check if legal
	// CURRENT: Enforece terrain height
	enforceOverTerrain();
	setPosition(m_position);
}

float3 Fruit::getHomePosition() const { return m_worldHome; }

void Fruit::setVelocity(float3 velo) { m_directionalVelocity = velo; }

Fruit::Fruit(float3 pos) : Entity() {
	setStartPosition(pos);
	setPosition(pos);
	m_worldHome = pos;
	m_nrOfFramePhases = 0;
	m_currentFramePhase = 0;
	m_frameTime = 0.0f;
}
