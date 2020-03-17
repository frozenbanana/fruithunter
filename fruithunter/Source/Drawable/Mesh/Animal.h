#pragma once
#include "Entity.h"

class Animal : public Entity {
private:
	int m_fruitType;
	int m_nrRequiredFruits;
	int m_nrFruitsTaken;
	float m_fruitRange;
	bool m_hasAttacked;
	bool m_isSatisfied;
	float m_playerRange;
	float m_throwStrength;

	float3 m_origin;
	float3 m_walkToPos;
	float3 m_sleepPos;	   // pos to go to when bribed
	float m_startRotation; // in y axis
	float m_chargeSpeed;
	float m_returnSpeed;
	float m_walkTimeTracker;

	// Thought bubble
	Entity m_thoughtBubble;
	float3 m_thoughtBubbleOffset;
	bool m_isLookedAt;

	void walkAndBack(float dt);
	void walkToSleep(float dt);
	void makeHappySound();
	void makeEatingSound();

public:
	void setAttacked(bool attacked);
	void makeAngrySound(); // is called in levelhandler
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
	void draw();
	void draw_onlyAnimal();
	void update(float dt, float3 playerPos);
	void beginWalk(float3 pos);
	bool checkLookedAt(float3 playerPos, float3 rayDir);
};
