#include "CollectionPoint.h"
#include "AudioHandler.h"

FileSyncer CollectionPoint::file;
float CollectionPoint::m_distanceThreshold = 1.5;
float CollectionPoint::m_acceleration_toPlayer = 30;
float CollectionPoint::m_startStrength = 10;
float CollectionPoint::m_slowdown = 0.1;

void CollectionPoint::setType(FruitType type) {
	m_type = type;
	switch (type) {
	case APPLE:
		m_explosion.load(ParticleSystem::Type::EXPLOSION_APPLE, 0, m_explosion_emitCount);
		m_sparkle.load(ParticleSystem::Type::SPARKLE_APPLE, m_sparkle_emitRate, 0);
		break;
	case BANANA:
		break;
	case MELON:
		break;
	case DRAGON:
		break;
	case NR_OF_FRUITS:
		break;
	default:
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

void CollectionPoint::load(float3 position, float3 velocity, FruitType type) {
	m_startPosition = position;
	m_position = position;
	m_velocity = velocity * m_startStrength;
	setType(type);
	m_explosion.setScale(m_explosion_spawnSize);
	m_sparkle.setScale(m_sparkle_spawnSize);
	//emit explosion
	m_explosion.setPosition(m_startPosition);//need to set position here, otherwise the particles will spawn at (0,0,0)
	m_explosion.emit(m_explosion_emitCount);
}

bool CollectionPoint::isFinished() const {
	return m_reachedDestination && (m_sparkle.activeParticleCount() == 0);
}

FruitType CollectionPoint::getFruitType() const { return m_type; }

bool CollectionPoint::update(float dt, float3 target) { 
	file.sync();
	m_explosion.update(dt);
	m_sparkle.update(dt);

	if (!m_reachedDestination) {
		float3 toTarget = Normalize(target - m_position);
		m_velocity += toTarget * m_acceleration_toPlayer * dt;
		m_velocity *= pow(m_slowdown, dt);
		m_position += m_velocity * dt;
		if ((m_position - target).Length() < m_distanceThreshold) {
			// inside threshold
			AudioHandler::getInstance()->playOnce(AudioHandler::COLLECT);
			m_sparkle.emitingState(false);
			m_reachedDestination = true;
			return true;
		}
	}
	return false;
}

void CollectionPoint::draw() {
	m_explosion.draw();

	m_sparkle.setPosition(m_position);
	m_sparkle.draw();
}
