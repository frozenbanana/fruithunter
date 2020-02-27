#pragma once
#include "GlobalNamespaces.h"

class Particle {
private:
	float3 m_position;
	float4 m_color;

public:
	Particle(float3 position = float3(0.0), float4 color = float4(1.0));

	void setPosition(float3 position);
	float3 getPosition() const;

	void setColor(float4 color);
	float4 getColor() const;

	void update(float dt, float3 velocity);
};