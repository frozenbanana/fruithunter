#pragma once
#include "Entity.h"
#include "ParticleSystem.h"
#include "Transformation.h"
#include "Fragment.h"
class TotemLock : public Transformation, public Fragment {
private:
	Entity m_obj_totem;
	ParticleSystem m_ps_collector;

	bool m_activated = false;
	float m_activationRadius = 3.f; // distance for player to interact

	//float3 m_removalPlaneColor = float3(1,0,0);
	//float m_removalTime = 3; // in seconds
	//float m_removalProgress = 0;

public:
	TotemLock(float3 position, float rotationY);

	void update(float dt);

	void draw();

};
