#include "Fruit.h"
#include "Input.h"
#include "PathFindingThread.h"


void Fruit::jump(float3 direction, float power) { m_velocity += power * direction; }

void Fruit::setStartPosition(float3 pos) {
	setPosition(pos);
	setWorldHome(pos);
	m_startAnimationPosition = pos;
	m_heightAnimationPosition = pos;
	m_destinationAnimationPosition = pos;
	m_nextDestinationAnimationPosition = pos;
}

void Fruit::setNextDestination(float3 nextDest) { m_nextDestinationAnimationPosition = nextDest; }

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
	if (atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {
		m_position.y = TerrainManager::getInstance()->getHeightFromPosition(m_position) +
					   abs(getHalfSizes().y / 2);
	}
}

void Fruit::setAnimationDestination() {
	/*m_destinationAnimationPosition = m_nextDestinationAnimationPosition;*/
	m_startAnimationPosition = getPosition();
	m_heightAnimationPosition =
		XMVectorLerp(m_startAnimationPosition, m_destinationAnimationPosition, 0.5f);
	m_heightAnimationPosition.y += 1.f;
}
void Fruit::setWorldHome(float3 pos) {
	m_worldHome = pos;
	m_worldHome.y = TerrainManager::getInstance()->getHeightFromPosition(pos);
}

bool Fruit::withinDistanceTo(float3 target, float treshhold) {
	return (m_position - target).Length() < treshhold;
}

void Fruit::update(float dt, float3 playerPosition, vector<shared_ptr<Entity>> collidables) {
	if (withinDistanceTo(playerPosition, 80.f)) {
		doBehavior(playerPosition);
		setDirection();
		updateAnimated(dt);
		updateVelocity(dt);
		move(dt);
		enforceOverTerrain();
		handleAvailablePath(m_position);
	}
}

void Fruit::move(float dt) {
	// m_speed.y = 0.0f;


	m_position += m_velocity * dt;
	setPosition(m_position);
}

float3 Fruit::getHomePosition() const { return m_worldHome; }



Fruit::Fruit(float3 pos) : Entity() {
	setStartPosition(pos);
	setPosition(pos);
	m_worldHome = pos;
	m_nrOfFramePhases = 0;
	m_currentFramePhase = 0;
	m_frameTime = 0.0f;
}

// Perhaps a useful function later.
void Fruit::behaviorInactive(float3 playerPosition) { return; }

void Fruit::setDirection() {
	auto pft = PathFindingThread::getInstance();
	// pft->m_mutex.lock();
	if (!m_availablePath.empty() &&
		atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {
		m_direction = m_availablePath.back() - m_position;
		m_direction.Normalize();
	}
	// pft->m_mutex.unlock();
}

void Fruit::behaviorReleased() {
	// TODO: Placeholder for later adding sound effects
	auto height = TerrainManager::getInstance()->getHeightFromPosition(m_position);

	if (atOrUnder(height)) {
		changeState(PASSIVE);
		stopMovement();
		m_afterRealease = false;
	}
}

void Fruit::updateVelocity(float dt) {
	m_velocity *= pow(m_friction / 60.f, dt);
	m_direction.Normalize();
	m_velocity += (m_direction * m_speed + m_gravity) * dt;
	// m_position += (m_directionalVelocity + m_speed) * dt;
	// m_velocity += float3(-m_friction, 0.0f, -m_friction) * dt;
}

void Fruit::stopMovement() {
	m_velocity = float3(0.f);
	m_speed = 0.f;
	m_availablePath.clear();
}

void Fruit::release(float3 direction) {
	changeState(RELEASED);
	stopMovement();
	m_direction = direction;
	m_speed = 40.f;
	m_velocity = m_direction * m_speed;
	m_speed = 0.f;
	m_afterRealease = true;
}
