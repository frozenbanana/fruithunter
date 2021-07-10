#pragma once
#include "Entity.h"
#include "ParticleSystem.h"
#include "EffectSystem.h"
class CollectionPoint : public Transformation {
private:
	FruitType m_type;
	Skillshot m_skillType;

	Entity m_fruit;
	const float m_fruit_rotationSpeed = 1;

	const float m_distanceThreshold = 1.5f;
	const float m_acceleration_toPlayer = 30.f;
	const float m_startStrength = 10.f;
	const float m_slowdown = 0.1f;
	float3 m_velocity;
	bool m_reachedDestination = false;

	EffectSystem m_effect_explosion, m_effect_sparkle, m_effect_stars;

	void setType(FruitType type);
	void setSkillType(Skillshot skillType);

public:
	CollectionPoint();
	~CollectionPoint();

	void load(float3 position, float3 velocity, FruitType type, Skillshot skillType);

	bool isFinished() const;
	FruitType getFruitType() const;

	bool update(float dt, float3 target);
	void draw();

};
