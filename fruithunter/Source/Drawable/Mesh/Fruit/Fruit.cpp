#include "Fruit.h"
#include "Input.h"
#include "AudioHandler.h"
#include "PathFindingThread.h"

#define LONGSHOT 25.f
#define MEDIUMSHOT 15.f
#define FASTMOVING_VELOCITY 11.f

void Fruit::jump(float3 direction, float power) { m_velocity += power * direction; }

void Fruit::setStartPosition(float3 pos) {
	setPosition(pos);
	setWorldHome(pos);
	m_startAnimationPosition = pos;
	m_heightAnimationPosition = pos;
	m_destinationAnimationPosition = pos;
	m_nextDestinationAnimationPosition = pos;

	m_particleSystem = make_unique<ParticleSystem>(ParticleSystem::STARS);
	m_particleSystem->setInActive();
}

void Fruit::setNextDestination(float3 nextDest) { m_nextDestinationAnimationPosition = nextDest; }

void Fruit::hit(float3 playerPos) {
	changeState(CAUGHT);
	float dist = (playerPos - m_position).Length();
	float4 colors[3];
	int nrOf = 5;
	if (dist > LONGSHOT) {
		if (!m_onGround || m_velocity.Length() > FASTMOVING_VELOCITY) {
			// gold
			colors[0] = float4(1.00f, 0.95f, 0.00f, 1.0f);
			colors[1] = float4(0.97f, 0.97f, 0.01f, 1.0f);
			colors[2] = float4(0.99f, 0.98f, 0.02f, 1.0f);
			nrOf = 22;
		}
		else {
			// gold
			colors[0] = float4(1.00f, 0.95f, 0.00f, 1.0f);
			colors[1] = float4(0.97f, 0.97f, 0.01f, 1.0f);
			colors[2] = float4(0.99f, 0.98f, 0.02f, 1.0f);
			nrOf = 12;
		}
	}
	else if (dist > MEDIUMSHOT) {
		if (!m_onGround || m_velocity.Length() > FASTMOVING_VELOCITY) {
			// case 2: Medium shot
			// in air or fast moving -> gold
			// Gold
			colors[0] = float4(1.00f, 0.95f, 0.00f, 1.0f);
			colors[1] = float4(0.97f, 0.97f, 0.01f, 1.0f);
			colors[2] = float4(0.99f, 0.98f, 0.02f, 1.0f);
			nrOf = 8;
		}
		else {
			// silver
			colors[0] = float4(0.75f, 0.75f, 0.75f, 1.0f);
			colors[1] = float4(0.75f, 0.75f, 0.75f, 1.0f);
			colors[2] = float4(0.75f, 0.75f, 0.75f, 1.0f);
			nrOf = 13;
		}
	}
	else {
		// bronze
		colors[0] = float4(0.69f, 0.34f, 0.05f, 1.0f);
		colors[1] = float4(0.71f, 0.36f, 0.07f, 1.0f);
		colors[2] = float4(0.70f, 0.32f, 0.09f, 1.0f);
		nrOf = 6;
	}
	m_particleSystem->setColors(colors);
	m_particleSystem->emit(nrOf);
	m_currentMaterial = 2;
}

int Fruit::getFruitType() { return m_fruitType; }

void Fruit::enforceOverTerrain() {
	if (atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {
		m_position.y = TerrainManager::getInstance()->getHeightFromPosition(m_position) +
					   abs(getHalfSizes().y / 2);
	}
}

void Fruit::checkOnGroundStatus() {
	m_onGround = atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position));
}

void Fruit::setAnimationDestination() {
	m_destinationAnimationPosition = m_nextDestinationAnimationPosition;
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

ParticleSystem* Fruit::getParticleSystem() { return m_particleSystem.get(); }

void Fruit::update(float dt, float3 playerPosition) {
	if (withinDistanceTo(playerPosition, 80.f)) {
		m_particleSystem->setPosition(m_position);
		checkOnGroundStatus();
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
	m_particleSystem->setAmountOfParticles(22);
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

	float friction = m_onGround ? m_groundFriction : m_airFriction;
	// friction = m_groundFriction;
	m_velocity *= pow(friction / 60.f, dt);
	m_direction.Normalize();
	float3 dir = m_direction * m_speed;
	m_velocity += (m_direction * m_speed + m_gravity) * dt;
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
	// m_speed = 40.f;
	m_velocity = m_direction * THROWVELOCITY;
	m_speed = 0.f;
	m_afterRealease = true;
}
