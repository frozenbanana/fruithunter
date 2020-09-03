#pragma once
#include "Entity.h"

class Animal : public Entity {
public:
	enum Type {
		Bear,
		Goat,
		Gorilla,
		Length
	};

private:
	Type m_type = Bear;
	FruitType m_fruitType = FruitType::APPLE;
	int m_nrRequiredFruits = 0;
	int m_nrFruitsTaken = 0;
	float m_fruitRange = 5;
	bool m_hasAttacked = false;
	bool m_isSatisfied = false;
	float m_playerRange = 5;
	float m_throwStrength = 5;

	float3 m_origin;
	float3 m_walkToPos;
	float3 m_sleepPos;	   // pos to go to when bribed
	float m_startRotation; // in y axis
	float m_chargeSpeed = 6;
	float m_returnSpeed = 1;
	float m_walkTimeTracker = 3;

	// Thought bubble
	Entity m_thoughtBubble;
	float3 m_thoughtBubbleOffset;
	bool m_isLookedAt = false;

	void walkAndBack(float dt);
	void walkToSleep(float dt);
	void makeHappySound();
	void makeEatingSound();

public:
	Animal(float3 position, float3 sleepPosition, Type type, FruitType fruitType,
		size_t nrRequiredFruits, float rotationY);
	~Animal();

	float getThrowStrength() const;
	float getPlayerRange() const;
	float getFruitRange() const;
	FruitType getfruitType() const;
	int getRequiredFruitCount() const;
	float3 getSleepPosition() const;
	Animal::Type getType() const;

	void setThrowStrength(float strength);
	void setPlayerRange(float range);
	void setFruitRange(float range);
	void setFruitType(FruitType type);
	void setRequiredFruitCount(int count);
	void setSleepPosition(float3 position);
	void setType(Animal::Type type);

	void makeAngrySound(); // is called in levelhandler
	
	void setAttacked(bool attacked);
	bool notBribed() const;
	void grabFruit(float3 pos);
	void beginWalk(float3 pos);
	bool checkLookedAt(float3 playerPos, float3 rayDir);
	
	void update(float dt, float3 playerPos);
	
	void draw();
	void draw_onlyAnimal();

	void reset();

};
