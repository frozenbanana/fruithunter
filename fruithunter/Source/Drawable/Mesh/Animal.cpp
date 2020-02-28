#include "Animal.h"

Animal::Animal(string modelName, float playerRange, float fruitRange, int fruitType,
	float throwStrength, float3 position, float rotation)
	: Entity(modelName, position) {
	m_playerRange = playerRange;
	m_fruitRange = fruitRange;
	m_fruitType = fruitType;
	m_throwStrength = throwStrength;
	m_origin = position;
	m_walkTimeTracker = 3;
	m_chargeSpeed = 6;
	m_returnSpeed = 1;
	rotateY(rotation);
}

Animal::~Animal() {}

float Animal::getThrowStrength() const { return m_throwStrength; }

float Animal::getPlayerRange() const { return m_playerRange; }

float Animal::getFruitRange() const { return m_fruitRange; }

void Animal::update(float dt) {
	if (m_walkTimeTracker < 2) {
		if (m_walkTimeTracker < 1) {
			setPosition(XMVectorLerp(m_origin, m_walkToPos, m_walkTimeTracker));
			m_walkTimeTracker += dt * m_chargeSpeed;
		}
		else {
			setPosition(XMVectorLerp(m_origin, m_walkToPos, 2 - m_walkTimeTracker));
			rotateZ(sin(m_walkTimeTracker * 10) * dt);
			m_walkTimeTracker += dt * m_returnSpeed;
		}
	}
}

void Animal::pushPlayer(float3 pos) {
	if (m_walkTimeTracker > 2) {
		m_walkToPos = (pos + getPosition()) * 0.5;
		m_walkTimeTracker = 0.f;
	}
}
