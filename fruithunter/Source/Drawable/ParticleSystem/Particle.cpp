#include "Particle.h"
#include "Renderer.h"

Particle::Particle(float3 position, float4 color, float size, float isActive) {
	m_position = position;
	m_color = color;
	m_size = size;
	m_isActive = isActive;
}

void Particle::setActiveValue(float isActive) { m_isActive = isActive; }
float Particle::getActiveValue() const { return m_isActive; }

float3 Particle::getPosition() const { return m_position; }
void Particle::setPosition(float3 position) { m_position = position; }

float4 Particle::getColor() const { return m_color; };
void Particle::setColor(float4 color) { m_color = color; }

float Particle::getSize() const { return m_size; }
void Particle::setSize(float size) { m_size = size; }

void Particle::update(float dt, float3 velocity) { m_position += velocity * dt; }
