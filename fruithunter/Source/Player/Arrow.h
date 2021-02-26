#pragma once
#include "Entity.h"
#include "ParticleSystem.h"
#include "TerrainBatch.h"
class Arrow : public Entity {
private:
	static const string m_model; // initilized in cpp file! 

	shared_ptr<ParticleSystem> m_trailEffect;

	float3 m_velocity = float3(0.);
	float3 m_oldVelocity = float3(0.);

	float m_arrowPitch = 0;
	float m_arrowYaw = 0;
	float m_maxTravelLengthSquared = 80.f * 80.f;

	// Assuming Fluid density * dragCoefficient * 0.5 = 1.
	float m_arrowArea = 0.0001f;
	float m_arrowMass = 0.1f;
	float m_arrowLength = 0.5f;
	float m_arrowRotation = 0.0f;

	bool m_active = false; // determines if the arrow will be updated or not

	//-- Private Functions --
	void update_physics(float dt, float3 windVector);
	void calcArea(float3 relativeWindVector);
	float calcAngle(float3 vec1, float3 vec2);

public:
	bool isActive() const;
	float3 getPosition_back() const;
	float3 getPosition_front() const;

	void setPosition_front(float3 position);
	void setPosition_back(float3 position);

	void collide_scene(float dt);
	bool collide_entity(float dt, Entity& entity);
	bool collide_terrainBatch(float dt, TerrainBatch& terrains);
	void collided(float3 target);

	void update(float dt);

	void initilize(float3 frontPosition, float3 velocity);
	void changeState(bool state);

	Arrow();
	~Arrow();

};
