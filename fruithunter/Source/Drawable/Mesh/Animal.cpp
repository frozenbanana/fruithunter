#include "Animal.h"
Animal::Animal(string modelName, float playerRange, float fruitRange, int fruitType,
	int nrRequiredFruits, float throwStrength, float3 position, float rotation)
	: Entity(modelName, position) {
	m_playerRange = playerRange;
	m_fruitRange = fruitRange;
	m_fruitType = fruitType;
	m_nrRequiredFruits = nrRequiredFruits;
	m_nrFruitsTaken = 0;
	m_throwStrength = throwStrength;
	m_origin = position;
	m_walkTimeTracker = 3;
	m_chargeSpeed = 6;
	m_returnSpeed = 1;
	rotateY(rotation);
	m_startRotation = rotation;
}

Animal::~Animal() {}

float Animal::getThrowStrength() const { return m_throwStrength; }

float Animal::getPlayerRange() const { return m_playerRange; }

float Animal::getFruitRange() const { return m_fruitRange; }

int Animal::getfruitType() const { return m_fruitType; }

void Animal::grabFruit(float3 pos) {
	beginWalk(pos);
	m_nrFruitsTaken++;
}

void Animal::update(float dt) {
	if (m_walkTimeTracker < 2) {
		if (m_walkTimeTracker < 1) { // on way to position
			setPosition(XMVectorLerp(m_origin, m_walkToPos, m_walkTimeTracker));
			m_walkTimeTracker += dt * m_chargeSpeed;
		}
		else if (m_nrFruitsTaken < m_nrRequiredFruits) { // on way back from position
			setPosition(XMVectorLerp(m_origin, m_walkToPos, 2 - m_walkTimeTracker));
			float3 vec =
				float3::Transform(float3::Forward, float4x4::CreateRotationY(m_startRotation));
			float angleSpeed = XM_PI * 3;
			float amplitude = 0.3f;
			setRotation(float3(
				0.f, m_startRotation, sin((m_walkTimeTracker - 1.f) * angleSpeed) * amplitude));
			m_walkTimeTracker += dt * m_returnSpeed;
		}
		else { // if(nrOfFruits >= requirement)// m_walkTimeTracker will go in interval [1,2]	//
			   // lays down
			setRotation(float3(XM_PI * (m_walkTimeTracker - 1.f), m_startRotation, 0.f));
			m_walkTimeTracker += dt;
		}
	}
	else if (m_nrFruitsTaken < m_nrRequiredFruits)
		setRotation(float3(0.f, m_startRotation, 0.0f));
}

void Animal::beginWalk(float3 pos) {
	if (m_walkTimeTracker >= 2) {
		m_walkToPos = (pos + getPosition()) * 0.5;
		m_walkTimeTracker = 0.f;
	}
}
