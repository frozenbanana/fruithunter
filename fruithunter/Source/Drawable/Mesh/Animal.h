#pragma once
#include "Entity.h"

class Animal : public Entity {
private:
	int m_fruitType;
	float m_fruitRange;
	float m_playerRange;
	float m_throwStrength;
	float3 m_origin;
	float3 m_walkToPos;
	float m_chargeSpeed;
	float m_returnSpeed;
	float m_walkTimeTracker;

public:
	Animal(string modelName, float playerRange, float fruitRange, int fruitType,
		float throwStrength, float3 position, float rotation);
	~Animal();

	float getThrowStrength() const;
	float getPlayerRange() const;
	float getFruitRange() const;

	void update(float dt);
	void pushPlayer(float3 pos);
};
