#include "Animal.h"

Animal::Animal(string modelName, float playerRange, float fruitRange, int fruitType,
	float throwStrength, float3 position, float rotation)
	: Entity(modelName, position) {
	m_playerRange = playerRange;
	m_fruitRange = fruitRange;
	m_fruitType = fruitType;
	m_throwStrength = throwStrength;
	rotateY(rotation);
}

Animal::~Animal() {}

float Animal::getThrowStrength() const { return m_throwStrength; }

float Animal::getPlayerRange() const { return m_playerRange; }

float Animal::getFruitRange() const { return m_fruitRange; }
