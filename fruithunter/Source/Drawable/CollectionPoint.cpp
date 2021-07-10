#include "CollectionPoint.h"
#include "AudioController.h"

void CollectionPoint::setType(FruitType type) {
	float fruit_scale = 1;
	m_type = type;
	switch (type) {
	case APPLE:
		m_fruit.load("Apple_000000");
		fruit_scale = 0.15f;

		m_effect_explosion.loadFromPreset("explosion_apple");
		m_effect_sparkle.loadFromPreset("sparkle_apple");
		break;
	case BANANA:
		m_fruit.load("Banana_000000");
		fruit_scale = 0.8f;

		m_effect_explosion.loadFromPreset("explosion_banana");
		m_effect_sparkle.loadFromPreset("sparkle_banana");
		break;
	case MELON:
		m_fruit.load("Melon_000000");
		fruit_scale = 0.15f;

		m_effect_explosion.loadFromPreset("explosion_melon");
		m_effect_sparkle.loadFromPreset("sparkle_melon");
		break;
	case DRAGON:
		m_fruit.load("Dragon_000000");
		fruit_scale = 0.15f;

		m_effect_explosion.loadFromPreset("explosion_dragon");
		m_effect_sparkle.loadFromPreset("sparkle_dragon");
		break;
	}
	m_fruit.setScale(fruit_scale);

	m_effect_explosion.setRotation(float3(-XM_PI / 2, 0, 0));
}

void CollectionPoint::setSkillType(Skillshot skillType) {
	switch (skillType) {
	case SS_GOLD:
		m_effect_stars.loadFromPreset("explosion_gold");
		break;
	case SS_SILVER:
		m_effect_stars.loadFromPreset("explosion_silver");
		break;
	case SS_BRONZE:
		m_effect_stars.loadFromPreset("explosion_bronze");
		break;
	}
	m_effect_stars.setRotation(float3(-XM_PI / 2, 0, 0)); // point up
}

CollectionPoint::CollectionPoint() {}

CollectionPoint::~CollectionPoint() {}

void CollectionPoint::load(float3 position, float3 velocity, FruitType type, Skillshot skillType) {
	setPosition(position);
	m_velocity = velocity * m_startStrength;
	setType(type);
	setSkillType(skillType);
	// emit explosion
	m_effect_stars.setPosition(position);
	m_effect_stars.burst();
	m_effect_explosion.setPosition(position);
	m_effect_explosion.burst();
}

bool CollectionPoint::isFinished() const {
	return m_reachedDestination && (m_effect_sparkle.getActiveParticleCount() == 0) &&
		   (m_effect_explosion.getActiveParticleCount() == 0) &&
		   (m_effect_stars.getActiveParticleCount() == 0);
}

FruitType CollectionPoint::getFruitType() const { return m_type; }

bool CollectionPoint::update(float dt, float3 target) {
	// update effects
	m_effect_explosion.update(dt);
	m_effect_sparkle.update(dt);
	m_effect_stars.update(dt);

	// rotate fruit
	m_fruit.rotate(float3(1.012f, 1.07f, 1.03f) * dt * m_fruit_rotationSpeed);

	// update sparkle movement
	if (!m_reachedDestination) {
		float3 toTarget = Normalize(target - getPosition());
		m_velocity += toTarget * m_acceleration_toPlayer * dt;
		m_velocity *= pow(m_slowdown, dt);
		move(m_velocity * dt);
		if ((getPosition() - target).Length() < m_distanceThreshold) {
			// inside threshold
			AudioController::getInstance()->play(
				"collected-item", AudioController::SoundType::Effect);
			m_effect_sparkle.setEmittingState(false);
			m_reachedDestination = true;
			return true;
		}
	}
	return false;
}

void CollectionPoint::draw() {
	if (!m_reachedDestination) {
		m_fruit.setPosition(getPosition());
		Renderer::getInstance()->setBlendState_NonPremultiplied();
		m_fruit.draw();
		Renderer::getInstance()->setBlendState_Opaque();
	}

	m_effect_explosion.draw();
	m_effect_stars.draw();
	m_effect_sparkle.setPosition(getPosition());
	m_effect_sparkle.draw();
}
