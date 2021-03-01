#pragma once
#include "GlobalNamespaces.h"

class BoundingBox2D {
private:
	float2 m_point_start, m_point_end;

public:
	float2 getSize() const;

	void set(float2 pStart, float2 pEnd);

	bool isInside(float2 point) const;

	BoundingBox2D(float2 pStart = float2(), float2 pEnd = float2());
};
class Transformation2D {
protected:
	float2 m_position;
	float2 m_scale = float2(1.);
	float m_rotation = 0;

public:
	float2 getPosition() const;
	float2 getScale() const;
	float getRotation() const;
	float2 getNormal() const;
	float4x4 getMatrix() const;

	void setPosition(float2 position);
	void setScale(float2 scale);
	void setScale(float scale);
	void setRotation(float rotation);

	void move(float2 movement);
	void rotate(float rotate);

	static Transformation2D transform(const Transformation2D& target, const Transformation2D& matrix);
	static float2 transform(const float2& target, const Transformation2D& matrix);

	Transformation2D(float2 position = float2(0.), float2 scale = float2(1.), float rotation = 0);
};
class Projectile : public Transformation2D {
private:
	float2 m_velocity;
	float m_rotationVelocity = 0;

	float2 m_acceleration; // accumulated forces for next update call, will reset after call
	float2 m_gravity;
	float m_friction = 1;
	float m_mass = 1;

public:
	float2 getVelocity() const;
	float getRotationVelocity() const;
	float2 getAcceleration() const;
	float2 getGravity() const;
	float getFriction() const;
	float getMass() const;

	void setFriction(float friction);
	void setAcceleration(float2 acceleration);
	void setGravity(float2 gravity);
	void setVelocity(float2 velocity);
	void setRotationVelocity(float rotVelocity);
	void setMass(float mass);

	void applyForce(float2 force);

	void update(float dt);

	Projectile(float2 position = float2(0.), float mass = 1);
};