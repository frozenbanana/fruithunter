#include "Fruit.h"
#include "Input.h"
#include "SceneManager.h"
#include "PathFindingThread.h"
#include "AudioController.h"

#define LONGSHOT 25.f
#define MEDIUMSHOT 15.f
#define FASTMOVING_VELOCITY 11.f

void Fruit::draw_fruit() {
	if (m_isVisible) {
		Renderer::getInstance()->setBlendState_NonPremultiplied();
		draw_animate();
		Renderer::getInstance()->setBlendState_Opaque();
		m_particleSystem.draw(true);
	}
}

void Fruit::draw_fruit_shadow() { draw_animate_onlyMesh(float3(0, 0, 0)); }

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

	m_particleSystem.load("stars bronze", 0, 25);
}

void Fruit::setNextDestination(float3 nextDest) { m_nextDestinationAnimationPosition = nextDest; }

Skillshot Fruit::hit(float3 playerPos) {
	Skillshot hitType = Skillshot::SS_BRONZE;
	if (m_currentState != CAUGHT) {
		float dist = (playerPos - getPosition()).Length();
		string psName = "confetti";
		int nrOf = 5;
		if (dist > LONGSHOT) {
			if (!m_onGround || m_velocity.Length() > FASTMOVING_VELOCITY) {
				// gold
				psName = "stars gold";
				nrOf = 22;
				hitType = SS_GOLD;
			}
			else {
				// gold
				psName = "stars gold";
				nrOf = 12;
				hitType = SS_GOLD;
			}
		}
		else if (dist > MEDIUMSHOT) {
			if (!m_onGround || m_velocity.Length() > FASTMOVING_VELOCITY) {
				// case 2: Medium shot
				// in air or fast moving -> gold
				// Gold
				psName = "stars gold";
				nrOf = 8;
				hitType = SS_GOLD;
			}
			else {
				// silver
				psName = "stars silver";
				nrOf = 13;
				hitType = SS_SILVER;
			}
		}
		else {
			// bronze
			psName = "stars bronze";
			nrOf = 6;
			hitType = SS_BRONZE;
		}
		m_particleSystem.setDesc(psName);
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
					 abs(getHalfSizes().y + 0.01f);
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

void Fruit::bindToEnvironment(Environment* terrain) { m_boundTerrain = terrain; }

bool Fruit::withinDistanceTo(float3 target, float treshhold) {
	return (getPosition() - target).Length() < treshhold;
}

ParticleSystem* Fruit::getParticleSystem() { return &m_particleSystem; }

void Fruit::update() {
	Scene* scene = SceneManager::getScene();
	float dt = scene->getDeltaTime();

	if (withinDistanceTo(scene->m_player->getPosition(), 500.f)) {
		m_isVisible = true;
		m_particleSystem.setPosition(getPosition());
		checkOnGroundStatus(); // checks if on ground
		updateAnimated(dt); // animation stuff
		updateVelocity(dt); // update velocity (slowdown and apply accelration)
		doBehavior();
		setDirection();		 // walk towards AI walk node
		updateRespawn();
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
	m_direction.Normalize();
	float3 dir = m_direction * m_speed;
	m_velocity += (m_direction * m_speed + m_gravity) * dt;
	m_velocity *= pow(friction / 60.f, dt);
}

void Fruit::stopMovement() {
	m_velocity = float3(0.f);
	m_speed = 0.f;
	m_availablePath.clear();
}

void Fruit::respawn() {
	if (!m_respawning) {
		// on respawn
		m_respawning = true;
		m_respawn_timer = m_respawn_timeMax;
		m_startScale = getScale().x;
	}
}

void Fruit::updateRespawn() {
	// -- FRUIT RESPAWNING -- //
	// Change into passive mode to automaticly respawn melon

	if (m_respawning) {
		// update
		float dt = SceneManager::getScene()->getDeltaTime();

		float th = m_respawn_timeMax / 2;
		if (m_respawn_timer >= th && m_respawn_timer - dt < th) {
			// find new respawn point
			if (m_boundTerrain != nullptr) {
				// spawn on bound terrain
				float3 sp = m_boundTerrain->getRandomSpawnPoint();
				setPosition(sp + float3(0.f, 1.f, 0.f) * (getHalfSizes().y + 0.1f));
			}
			else {
				int tIndex =
					SceneManager::getScene()->m_terrains.getTerrainIndexFromPosition(getPosition());
				if (tIndex == -1) {
					// pick random terrain if not on a terrain (Plan B)
					tIndex = rand() % SceneManager::getScene()->m_terrains.size();
				}
				if (tIndex != -1) {
					float3 sp = SceneManager::getScene()->m_terrains[tIndex]->getRandomSpawnPoint();
					setPosition(sp + float3(0.f, 1.f, 0.f) * (getHalfSizes().y + 0.1f));
				}
				else {
					// this should never happen as fruits only can spawn if there is a terrain to
					// spawn from
					ErrorLogger::logError(
						"(Fruit) Fruit cant respawn. No terrains exists!", HRESULT());
				}
			}
			m_velocity *= 0;
		}
		m_respawn_timer = Clamp<float>(m_respawn_timer - dt, 0, m_respawn_timeMax);

		// scaling
		float factor = abs((m_respawn_timeMax / 2) - m_respawn_timer) / (m_respawn_timeMax / 2);
		setScale(m_startScale * factor);

		if (m_respawn_timer == 0) {
			// end of respawn
			m_respawning = false;
		}
	}
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
