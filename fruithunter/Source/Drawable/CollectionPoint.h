#pragma once
#include "Entity.h"
#include "ParticleSystem.h"
#include "VariableSyncer.h"
class CollectionPoint {
private:
	static FileSyncer file;
	FruitType m_type;
	Skillshot m_skillType;
	const size_t m_explosion_emitCount = 300;
	const size_t m_sparkle_emitRate = 100;
	const size_t m_stars_emitCount = 15;
	ParticleSystem m_explosion, m_sparkle, m_stars;
	Entity m_fruit;
	const float m_fruit_rotationSpeed = 1;

	static float m_distanceThreshold;
	static float m_acceleration_toPlayer;
	static float m_startStrength;
	static float m_slowdown;
	const float3 m_sparkle_spawnSize = float3(0.3);
	const float3 m_explosion_spawnSize = float3(1.);
	float3 m_startPosition;
	float3 m_position;
	float3 m_velocity;
	bool m_reachedDestination = false;

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
