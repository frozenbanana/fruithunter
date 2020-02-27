#include "Particle.h"
#include "Renderer.h"

Particle::Particle(float3 position, float4 color) {
	m_position = position;
	m_color = color;
}

float3 Particle::getPosition() const { return m_position; }
void Particle::setPosition(float3 position) { m_position = position; }

float4 Particle::getColor() const { return m_color; };
void Particle::setColor(float4 color) { m_color = color; }

void Particle::update(float dt, float3 velocity) { m_position += velocity * dt; }
