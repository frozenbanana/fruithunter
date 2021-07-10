#pragma once
#include "GlobalNamespaces.h"

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

	void imgui_properties();

	Transformation2D(float2 position = float2(0.), float2 scale = float2(1.), float rotation = 0);
};

class BoundingBox2D {
private:
	Matrix m_matrix;
	float2 m_size, m_alignment;

public:
	void set(const Matrix& matrix, float2 size, float2 alignment);

	float2 getCenter() const;
	vector<float2> getPoints() const;

	bool isInside(float2 point) const;
};

enum HorizontalAlignment { AlignLeft = -1, AlignMiddle = 0, AlignRight = 1 };
enum VerticalAlignment { AlignTop = -1, AlignCenter = 0, AlignBottom = 1 };

class Drawable2D : public Transformation2D {
private:
protected:
	bool m_drawing = true;
	HorizontalAlignment m_horizontalAligment = HorizontalAlignment::AlignMiddle;
	VerticalAlignment m_verticalAlignment = VerticalAlignment::AlignCenter;

	virtual void _draw(const Transformation2D& source) = 0;
	virtual void _imgui_properties();

public:
	void draw();
	void draw(const Transformation2D& matrix);

	bool isDrawing() const;
	virtual float2 getLocalSize() const = 0;
	float2 getSize() const;
	BoundingBox2D getBoundingBox() const;
	BoundingBox2D getBoundingBox(Matrix parentMatrices) const;

	void setDrawState(bool state);
	void setSize(float2 size);
	void setAlignment(
		HorizontalAlignment horizontal = AlignMiddle, VerticalAlignment vertical = AlignCenter);
	HorizontalAlignment getHorizontalAlignment() const;
	VerticalAlignment getVerticalAlignment() const;
	float2 getAlignment() const;

	void imgui_properties();
};

class Projectile2D : public Transformation2D {
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

	Projectile2D(float2 position = float2(0.), float mass = 1);
};