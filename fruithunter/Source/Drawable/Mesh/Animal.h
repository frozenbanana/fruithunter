#pragma once
#include "Entity.h"

class Animal : public Entity {
private:
	int m_fruitType;
	float m_fruitRange;
	float m_playerRange;
	float m_throwStrength;


public:
	Animal(string modelName, float playerRange, float fruitRange, int fruitType,
		float throwStrength, float3 position, float rotation);
	~Animal();

	float getThrowStrength() const;
	float getPlayerRange() const;
	float getFruitRange() const;
};
