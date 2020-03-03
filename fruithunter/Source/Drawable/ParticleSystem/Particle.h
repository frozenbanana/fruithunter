
#pragma once
#include "GlobalNamespaces.h"

class Particle {
private:
	float3 m_position;
	float4 m_color;
	float m_size;
	float m_isActive;

public:
	Particle(float3 position = float3(0.0), float4 color = float4(1.0), float size = 1.0f,
		float isActive = 0.0f);

	void setIsActive(float isActive);
	float getIsActive() const;

	void setPosition(float3 position);
	float3 getPosition() const;

	void setColor(float4 color);
	float4 getColor() const;

	float getSize() const;
	void setSize(float size);

	void update(float dt, float3 velocity);
};