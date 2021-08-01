#include "Arrow.h"
#include "SceneManager.h"
#include "AudioController.h"

const string Arrow::m_model = "ArrowV3";

void Arrow::update_physics(float dt, float3 windVector) {
	// Update acceleration
	float3 relativeVelocity;
	if (windVector.Length() < 0.0001f) {
		relativeVelocity = m_velocity;
		m_arrowArea = 0.0001f;
	}
	else {
		relativeVelocity = m_velocity + windVector;
		calcArea(relativeVelocity);
	}

	float totalDragTimesLength = -m_arrowArea * relativeVelocity.Length() / m_arrowMass;

	float3 acceleration = float3(totalDragTimesLength * relativeVelocity.x,
		(totalDragTimesLength * relativeVelocity.y) - 9.82f,
		totalDragTimesLength * relativeVelocity.z);

	m_velocity += (acceleration + windVector) * dt;

	float angle = calcAngle(m_velocity, m_oldVelocity);
	m_arrowPitch += angle;

	m_oldVelocity = m_velocity;

}

void Arrow::calcArea(float3 relativeWindVector) {
	float angle = calcAngle(relativeWindVector, m_velocity);
	m_arrowArea = ((1 - sin(angle)) * 0.0001f) + (sin(angle) * 0.005f);
}

float Arrow::calcAngle(float3 vec1, float3 vec2) {
	float3 normalisedVec1 = vec1;
	normalisedVec1.Normalize();

	float3 normalisedVec2 = vec2;
	normalisedVec2.Normalize();
	float soonAngle = normalisedVec1.Dot(normalisedVec2);
	soonAngle = min(1.f, max(-1.f, soonAngle)); // clamped to avoid NaN in cos
	return acos(soonAngle);
}

float3 Arrow::getPosition_back() const {
	float3 dir = m_velocity;
	dir.Normalize();
	/*
	 * Back of arrow!
	 * Point depends on velocity!
	 */
	return getBoundingBoxPos() - dir * getHalfSizes().z;
}

float3 Arrow::getPosition_front() const { 
	float3 dir = m_velocity;
	dir.Normalize();
	/*
	 * Front of arrow!
	 * Minus 10% of width!
	 * Point depends on velocity!
	 */
	return getBoundingBoxPos() + dir * getHalfSizes().z * 0.9f;
}

void Arrow::setPosition_front(float3 position) {
	setPosition(position - (getPosition_front() - getPosition())); 
}

void Arrow::setPosition_back(float3 position) {
	setPosition(position + (getPosition_front() - getPosition()));
}

bool Arrow::isActive() const { return m_active; }

void Arrow::collide_scene(float dt) {
	//collide terrainBatch
	collide_terrainBatch(dt, SceneManager::getScene()->m_terrains);
	//collide entities (culled by position)
	vector<shared_ptr<Entity>*> entities =
		SceneManager::getScene()->m_entities.getElementsByPosition(getPosition());
	for (size_t i = 0; i < entities.size(); i++) {
		collide_entity(dt, **(entities[i]));
	}
}

bool Arrow::collide_entity(float dt, Entity& entity) { 
	float3 pos = getPosition_front(); 
	float3 vel = m_velocity * dt;
	float rayDist = 0;
	float3 intersection, normal;
	if (entity.castRayEx_limitDistance(pos, vel, intersection, normal)) {
		collided(intersection);
		return true;
	}
	return false;
}

bool Arrow::collide_terrainBatch(float dt, TerrainBatch& terrains) { 
	float3 pos = getPosition_front();
	float3 vel = m_velocity * dt;
	float ray = terrains.castRay(pos, vel);
	if (ray != -1) {
		float3 target = pos + vel * ray;
		collided(target);
		return true;
	}
	return false;
}

void Arrow::collided(float3 target) {
	setPosition_front(target);
	changeState(false);
	SoundID id = AudioController::getInstance()->play("hit-wood", AudioController::SoundType::Effect);
	AudioController::getInstance()->scaleVolumeByDistance(
		id, (SceneManager::getScene()->m_camera.getPosition() - target).Length());
}

void Arrow::update(float dt) {
	if (m_active) {
		// dont update arrow position if arrow is disabled (hitting something makes it stop!)
		float3 wind(0.);
		Environment* inEnvironment =
			SceneManager::getScene()->m_terrains.getTerrainFromPosition(getPosition());
		if (inEnvironment != nullptr)
			wind = inEnvironment->getWindStatic();
		update_physics(dt, wind);
		//update position
		setPosition(getPosition() + m_velocity * dt);
		// angle
		lookAt(getPosition() + m_velocity);
	}
	//update trail
	//if (m_trailEffect.get() != nullptr) {
	//	m_trailEffect->setPosition(getPosition_back());
	//}
	if (m_trail.get())
		m_trail->setPosition(getPosition_back());
}

void Arrow::initilize(float3 frontPosition, float3 velocity) { 
	setPosition_front(frontPosition);
	m_velocity = velocity;
	lookAt(getPosition() + m_velocity);
	changeState(true);
}

void Arrow::changeState(bool state) {
	m_active = state;
	if (state == true) {
		// create trail effect when activated
		m_trail = make_shared<EffectSystem>();
		m_trail->loadFromPreset("arrow_glitter");
		SceneManager::getScene()->m_arrowParticles.push_back(m_trail);
	}
	else {
		// turn off trail effect (will be deleted when empty)
		if (m_trail.get()) {
			m_trail->setEmittingState(false);
			m_trail->markForDeletion();
			m_trail.reset(); // delete local pointer
		}
	}
}

Arrow::Arrow() : Entity(m_model, float3(0.), float3(0.5, 0.5, 0.5)) { }

Arrow::~Arrow() { changeState(false); }
