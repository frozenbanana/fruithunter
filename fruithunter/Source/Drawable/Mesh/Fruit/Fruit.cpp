#include "Fruit.h"
#include "Input.h"
#include "SceneManager.h"
#include "PathFindingThread.h"
#include "AudioController.h"

#define LONGSHOT 25.f
#define MEDIUMSHOT 15.f
#define FASTMOVING_VELOCITY 11.f

void Fruit::jump(float3 direction, float power) { 
	m_velocity += power * direction; 
}

void Fruit::setStartPosition(float3 pos) {
	setPosition(pos);
	setWorldHome(pos);
	m_startAnimationPosition = pos;
	m_heightAnimationPosition = pos;
	m_destinationAnimationPosition = pos;
	m_nextDestinationAnimationPosition = pos;

	m_particleSystem.load(ParticleSystem::Type::STARS_BRONZE, 0, 25);
}

void Fruit::setNextDestination(float3 nextDest) { m_nextDestinationAnimationPosition = nextDest; }

Skillshot Fruit::hit(float3 playerPos) {
	Skillshot hitType = SS_NOTHING;
	if (m_currentState != CAUGHT) {
		changeState(CAUGHT);
		float dist = (playerPos - getPosition()).Length();
		ParticleSystem::Type type = ParticleSystem::Type::CONFETTI;
		int nrOf = 5;
		if (dist > LONGSHOT) {
			if (!m_onGround || m_velocity.Length() > FASTMOVING_VELOCITY) {
				// gold
				type = ParticleSystem::Type::STARS_GOLD;
				nrOf = 22;
				hitType = SS_GOLD;
			}
			else {
				// gold
				type = ParticleSystem::Type::STARS_GOLD;
				nrOf = 12;
				hitType = SS_GOLD;
			}
		}
		else if (dist > MEDIUMSHOT) {
			if (!m_onGround || m_velocity.Length() > FASTMOVING_VELOCITY) {
				// case 2: Medium shot
				// in air or fast moving -> gold
				// Gold
				type = ParticleSystem::Type::STARS_GOLD;
				nrOf = 8;
				hitType = SS_GOLD;
			}
			else {
				// silver
				type = ParticleSystem::Type::STARS_SILVER;
				nrOf = 13;
				hitType = SS_SILVER;
			}
		}
		else {
			// bronze
			type = ParticleSystem::Type::STARS_BRONZE;
			nrOf = 6;
			hitType = SS_BRONZE;
		}
		m_particleSystem.setType(type);
		m_particleSystem.emit(nrOf);
		m_currentMaterial = hitType;
	}
	return hitType;
}

FruitType Fruit::getFruitType() { return m_fruitType; }

shared_ptr<Fruit> Fruit::createFruitFromType(FruitType type) { 
	shared_ptr<Fruit> fruit;
	switch (type) {
	case APPLE:
		fruit = make_shared<Apple>();
		break;
	case BANANA:
		fruit = make_shared<Banana>();
		break;
	case MELON:
		fruit = make_shared<Melon>();
		break;
	case DRAGON:
		fruit = make_shared<DragonFruit>();
		break;
	}
	return fruit;
}

void Fruit::enforceOverTerrain() {
	if (atOrUnder(SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition()))) {
		float newY = SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition()) +
					 abs(getHalfSizes().y / 2);
		setPosition(float3(getPosition().x, newY, getPosition().z));
	}
}

void Fruit::checkOnGroundStatus() {
	m_onGround =
		atOrUnder(SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition()));
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
	m_worldHome.y = SceneManager::getScene()->m_terrains.getHeightFromPosition(pos);
}

bool Fruit::withinDistanceTo(float3 target, float treshhold) {
	return (getPosition() - target).Length() < treshhold;
}

ParticleSystem* Fruit::getParticleSystem() { return &m_particleSystem; }

void Fruit::update() {
	Scene* scene = SceneManager::getScene();
	float dt = scene->getDeltaTime();

	if (withinDistanceTo(scene->m_player->getPosition(), 160.f)) {
		m_isVisible = true;
		m_particleSystem.setPosition(getPosition());
		checkOnGroundStatus(); // checks if on ground
		/*
		 * Flee: move towards edge of passive zone, away from player
		 * Passive: 
		        - If at water level, then jump towards home blindly
				- If not at home, then set AI to home
				- If failed to go home then set new home at current location
				- If at home, then just jump straight up. But after 2 jumps move home
		 * Caught: Jump straight up. Move straight to player
		*/
		doBehavior();
		setDirection(); // walk towards AI walk node
		updateAnimated(dt); // animation stuff
		updateVelocity(dt); // update velocity (slowdown and apply accelration)
		move(dt);			// update position from velocity
		enforceOverTerrain();// force fruit above ground
		handleAvailablePath(getPosition()); // Keeps track of next AI node to go to (discards nodes if to close)
	}
	else
		m_isVisible = false;
}

void Fruit::move(float dt) {
	// m_speed.y = 0.0f;
	setPosition(getPosition() + m_velocity * dt);
}

float3 Fruit::getHomePosition() const { return m_worldHome; }



Fruit::Fruit(float3 pos) : Entity() {
	setStartPosition(pos);
	setPosition(pos);
	setWorldHome(pos);
	m_nrOfFramePhases = 0;
	m_currentFramePhase = 0;
	m_frameTime = 0.0f;
}

// Perhaps a useful function later.
void Fruit::behaviorInactive() { return; }

void Fruit::setDirection() {
	auto pft = PathFindingThread::getInstance();
	// pft->m_mutex.lock();
	if (!m_availablePath.empty() &&
		m_onGround) {
		m_direction = m_availablePath.back() - getPosition();
		m_direction.Normalize();
	}
	// pft->m_mutex.unlock();
}

void Fruit::behaviorReleased() {
	// TODO: Placeholder for later adding sound effects
	auto height = SceneManager::getScene()->m_terrains.getHeightFromPosition(getPosition());

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

bool Fruit::rayCastWorld(float3 point, float3 forward, float3& intersection, float3& normal) {
	Scene* scene = SceneManager::getScene();

	float t = -1;
	// find terrain collision
	t = scene->m_terrains.castRay(point, forward);
	if (t > -1) {
		intersection = point + forward * t;
		normal = scene->m_terrains.getNormalFromPosition(intersection);
	}
	// object collisions
	CubeBoundingBox bb(vector<float3>({ point, point + forward }));
	vector<shared_ptr<Entity>*> entities = scene->m_entities.cullElements(bb);
	for (size_t i = 0; i < entities.size(); i++) {
		float3 cast_target, cast_normal;
		if ((*entities[i])->getIsCollidable()) {
			if ((*entities[i])->castRayEx_limitDistance(point, forward, cast_target, cast_normal)) {
				float tt = (cast_target - point).Length() / forward.Length();
				if (tt <= 1 && (t == -1 || tt < t)) {
					t = tt;
					intersection = cast_target;
					normal = cast_normal;
				}
			}
		}
	}
	return (t > -1);
}

bool Fruit::isOnGround(float3 position, float heightThreshold) {
	Scene* scene = SceneManager::getScene();
	float3 forward = float3(0, -1, 0) * heightThreshold;
	float t = -1;
	// find terrain height
	float h = scene->m_terrains.getHeightFromPosition(position);
	if (abs(position.y - h) < heightThreshold)
		return true;
	// object collisions
	vector<shared_ptr<Entity>*> entities = scene->m_entities.getElementsByPosition(position);
	for (size_t i = 0; i < entities.size(); i++) {
		float3 cast_target, cast_normal;
		if ((*entities[i])->getIsCollidable()) {
			if ((*entities[i])
					->castRayEx_limitDistance(position, forward, cast_target, cast_normal)) {
				// hit
				return true;
			}
		}
	}
	return false;
}

bool Fruit::isVisible() const { return m_isVisible; }

void Fruit::release(float3 direction) {
	changeState(RELEASED);
	stopMovement();
	m_direction = direction;
	// m_speed = 40.f;
	m_velocity = m_direction * THROWVELOCITY;
	m_speed = 0.f;
	m_afterRealease = true;
}
