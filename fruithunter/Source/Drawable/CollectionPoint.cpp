#include "CollectionPoint.h"
#include "AudioController.h"

FileSyncer CollectionPoint::file;
float CollectionPoint::m_distanceThreshold = 1.5;
float CollectionPoint::m_acceleration_toPlayer = 30;
float CollectionPoint::m_startStrength = 10;
float CollectionPoint::m_slowdown = 0.1;

void CollectionPoint::setType(FruitType type) {
	float fruit_scale = 1;
	m_type = type;
	switch (type) {
	case APPLE:
		m_explosion.load(ParticleSystem::Type::EXPLOSION_APPLE, 0, m_explosion_emitCount);
		m_sparkle.load(ParticleSystem::Type::SPARKLE_APPLE, m_sparkle_emitRate, 0);
		m_fruit.load("Apple_000000");
		fruit_scale = 0.15;
		break;
	case BANANA:
		m_explosion.load(ParticleSystem::Type::EXPLOSION_BANANA, 0, m_explosion_emitCount);
		m_sparkle.load(ParticleSystem::Type::SPARKLE_BANANA, m_sparkle_emitRate, 0);
		m_fruit.load("Banana_000000");
		fruit_scale = 0.8;
		break;
	case MELON:
		m_explosion.load(ParticleSystem::Type::EXPLOSION_MELON, 0, m_explosion_emitCount);
		m_sparkle.load(ParticleSystem::Type::SPARKLE_MELON, m_sparkle_emitRate, 0);
		m_fruit.load("Melon_000000");
		fruit_scale = 0.15;
		break;
	case DRAGON:
		m_explosion.load(ParticleSystem::Type::EXPLOSION_DRAGON, 0, m_explosion_emitCount);
		m_sparkle.load(ParticleSystem::Type::SPARKLE_DRAGON, m_sparkle_emitRate, 0);
		m_fruit.load("Dragon_000000");
		fruit_scale = 0.15;
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
		m_stars.load(ParticleSystem::Type::EXPLOSION_GOLD, 0, m_stars_emitCount);
		break;
	case SS_SILVER:
		m_stars.load(ParticleSystem::Type::EXPLOSION_SILVER, 0, m_stars_emitCount);
		break;
	case SS_BRONZE:
		m_stars.load(ParticleSystem::Type::EXPLOSION_BRONZE, 0, m_stars_emitCount);
		break;
	}
}

CollectionPoint::CollectionPoint() {
	if (!file.fileCreated()) {
		file.bind("distance:f", &m_distanceThreshold);
		file.bind("acceleration:f", &m_acceleration_toPlayer);
		file.bind("startStrength:f", &m_startStrength);
		file.bind("slowdown:f", &m_slowdown);
		file.connect("collectionPoint.txt");
	}
}

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
	return m_reachedDestination && (m_sparkle.activeParticleCount() == 0);
}

FruitType CollectionPoint::getFruitType() const { return m_type; }

bool CollectionPoint::update(float dt, float3 target) { 
	file.sync();
	//update effects
	m_explosion.update(dt);
	m_sparkle.update(dt);
	m_stars.update(dt);

	//rotate fruit
	m_fruit.rotate(float3(1.012, 1.07, 1.03) * dt * m_fruit_rotationSpeed);

	//update sparkle movement
	if (!m_reachedDestination) {
		float3 toTarget = Normalize(target - m_position);
		m_velocity += toTarget * m_acceleration_toPlayer * dt;
		m_velocity *= pow(m_slowdown, dt);
		m_position += m_velocity * dt;
		if ((m_position - target).Length() < m_distanceThreshold) {
			// inside threshold
			AudioController::getInstance()->play("collected-item", AudioController::SoundType::Effect);
			m_sparkle.emitingState(false);
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
