#include "CollectionPoint.h"
#include "AudioController.h"

float CollectionPoint::m_distanceThreshold = 1.5f;
float CollectionPoint::m_acceleration_toPlayer = 30.0f;
float CollectionPoint::m_startStrength = 10.0f;
float CollectionPoint::m_slowdown = 0.1f;

void CollectionPoint::setType(FruitType type) {
	float fruit_scale = 1;
	m_type = type;
	switch (type) {
	case APPLE:
		m_explosion.load("explosion apple", 0, m_explosion_emitCount);
		m_sparkle.load("sparkle apple", (float)m_sparkle_emitRate, size_t(0));
		m_fruit.load("Apple_000000");
		fruit_scale = 0.15f;
		break;
	case BANANA:
		m_explosion.load("explosion banana", 0, m_explosion_emitCount);
		m_sparkle.load("sparkle banana", (float)m_sparkle_emitRate, size_t(0));
		m_fruit.load("Banana_000000");
		fruit_scale = 0.8f;
		break;
	case MELON:
		m_explosion.load("explosion melon", 0, m_explosion_emitCount);
		m_sparkle.load("sparkle melon", (float)m_sparkle_emitRate, size_t(0));
		m_fruit.load("Melon_000000");
		fruit_scale = 0.15f;
		break;
	case DRAGON:
		m_explosion.load("explosion dragon", 0, m_explosion_emitCount);
		m_sparkle.load("sparkle dragon", (float)m_sparkle_emitRate, size_t(0));
		m_fruit.load("Dragon_000000");
		fruit_scale = 0.15f;
		break;
	}
	m_fruit.setScale(fruit_scale);
	m_explosion.setScale(m_explosion_spawnSize);
	m_stars.setScale(m_explosion_spawnSize);
	m_sparkle.setScale(m_sparkle_spawnSize);
}

void CollectionPoint::setSkillType(Skillshot skillType) {
	switch (skillType) {
	case SS_GOLD:
		m_stars.load("explosion gold", 0, m_stars_emitCount);
		break;
	case SS_SILVER:
		m_stars.load("explosion silver", 0, m_stars_emitCount);
		break;
	case SS_BRONZE:
		m_stars.load("explosion bronze", 0, m_stars_emitCount);
		break;
	}
}

CollectionPoint::CollectionPoint() {}

CollectionPoint::~CollectionPoint() {}

void CollectionPoint::load(float3 position, float3 velocity, FruitType type, Skillshot skillType) {
	m_startPosition = position;
	m_position = position;
	m_velocity = velocity * m_startStrength;
	setType(type);
	setSkillType(skillType);
	//emit explosion
	m_explosion.setPosition(m_startPosition);//need to set position here, otherwise the particles will spawn at (0,0,0)
	m_explosion.emit(m_explosion_emitCount);
	m_stars.setPosition(m_startPosition);
	m_stars.emit(m_stars_emitCount);
}

bool CollectionPoint::isFinished() const {
	return m_reachedDestination && (m_sparkle.getActiveParticleCount() == 0);
}

FruitType CollectionPoint::getFruitType() const { return m_type; }

bool CollectionPoint::update(float dt, float3 target) { 
	//update effects
	m_explosion.update(dt);
	m_sparkle.update(dt);
	m_stars.update(dt);

	//rotate fruit
	m_fruit.rotate(float3(1.012f, 1.07f, 1.03f) * dt * m_fruit_rotationSpeed);

	//update sparkle movement
	if (!m_reachedDestination) {
		float3 toTarget = Normalize(target - m_position);
		m_velocity += toTarget * m_acceleration_toPlayer * dt;
		m_velocity *= pow(m_slowdown, dt);
		m_position += m_velocity * dt;
		if ((m_position - target).Length() < m_distanceThreshold) {
			// inside threshold
			AudioController::getInstance()->play("collected-item", AudioController::SoundType::Effect);
			m_sparkle.setEmitingState(false);
			m_reachedDestination = true;
			return true;
		}
	}
	return false;
}

void CollectionPoint::draw() {
	if (!m_reachedDestination) {
		m_fruit.setPosition(m_position);
		Renderer::getInstance()->setBlendState_NonPremultiplied();
		m_fruit.draw();
		Renderer::getInstance()->setBlendState_Opaque();
	}
	m_explosion.draw();

	m_stars.draw();

	m_sparkle.setPosition(m_position);
	m_sparkle.draw();
}
