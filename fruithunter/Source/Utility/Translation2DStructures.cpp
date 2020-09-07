#include "Translation2DStructures.h"

Transformation2D::Transformation2D(float2 position, float2 scale, float rotation) {
	m_position = position;
	m_scale = scale;
	m_rotation = rotation;
}

float2 Transformation2D::getPosition() const { return m_position; }

float2 Transformation2D::getScale() const { return m_scale; }

float Transformation2D::getRotation() const { return m_rotation; }

float2 Transformation2D::getNormal() const { return float2(cos(m_rotation), sin(m_rotation)); }

void Transformation2D::setPosition(float2 position) { m_position = position; }

void Transformation2D::setScale(float2 scale) { m_scale = scale; }

void Transformation2D::setRotation(float rotation) { m_rotation = rotation; }

void Transformation2D::move(float2 movement) { m_position += movement; }

void Transformation2D::rotate(float rotate) { m_rotation += rotate; }

Projectile::Projectile(float2 position, float mass) : Transformation2D(position) { setMass(mass); }

float2 Projectile::getVelocity() const { return m_velocity; }

float Projectile::getRotationVelocity() const { return m_rotationVelocity; }

float2 Projectile::getAcceleration() const { return m_acceleration; }

float2 Projectile::getGravity() const { return m_gravity; }

float Projectile::getFriction() const { return m_friction; }

float Projectile::getMass() const { return m_mass; }

void Projectile::setFriction(float friction) { m_friction = friction; }

void Projectile::setAcceleration(float2 acceleration) { m_acceleration = acceleration; }

void Projectile::setGravity(float2 gravity) { m_gravity = gravity; }

void Projectile::setVelocity(float2 velocity) { m_velocity = velocity; }

void Projectile::setRotationVelocity(float rotVelocity) { m_rotationVelocity = rotVelocity; }

void Projectile::setMass(float mass) { m_mass = mass; }

void Projectile::applyForce(float2 force) { m_acceleration += force / m_mass; }

void Projectile::update(float dt) {
	//position, velocity, acceleration
	m_velocity += (m_acceleration+m_gravity) * dt;
	m_position += m_velocity * dt;
	m_velocity *= pow(m_friction, dt);
	m_acceleration = float2(0.);

	//rotation
	m_rotation += m_rotationVelocity * dt;

}
