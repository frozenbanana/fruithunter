#pragma once
#include "Entity.h"

class Animal : public Entity {
private:
	int m_fruitType;
	int m_nrRequiredFruits;
	int m_nrFruitsTaken;
	float m_fruitRange;

	float m_playerRange;
	float m_throwStrength;

	float3 m_origin;
	float3 m_walkToPos;
	float3 m_sleepPos;	   // pos to go to when bribed
	float m_startRotation; // in y axis
	float m_chargeSpeed;
	float m_returnSpeed;
	float m_walkTimeTracker;

	void walkAndBack(float dt);
	void walkToSleep(float dt);

public:
	Animal(string modelName, float playerRange, float fruitRange, int fruitType,
		int nrRequiredFruits, float throwStrength, float3 position, float3 sleepPos,
		float rotation);
	~Animal();

	float getThrowStrength() const;
	float getPlayerRange() const;
	float getFruitRange() const;
	int getfruitType() const;
	bool notBribed() const;

	void grabFruit(float3 pos);

	void update(float dt);
	void beginWalk(float3 pos);
};
