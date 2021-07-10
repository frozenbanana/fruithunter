#include "Translation2DStructures.h"
#include "ErrorLogger.h"

Transformation2D::Transformation2D(float2 position, float2 scale, float rotation) {
	m_position = position;
	m_scale = scale;
	m_rotation = rotation;
}

float2 Transformation2D::getPosition() const { return m_position; }

float2 Transformation2D::getScale() const { return m_scale; }

float Transformation2D::getRotation() const { return m_rotation; }

float2 Transformation2D::getNormal() const { return float2(cos(m_rotation), sin(m_rotation)); }

float4x4 Transformation2D::getMatrix() const {
	float4x4 m = float4x4::CreateScale(m_scale.x, m_scale.y, 1) *
				 float4x4::CreateRotationZ(m_rotation) *
				 float4x4::CreateTranslation(m_position.x, m_position.y, 0);
	return m;
}

void Transformation2D::setPosition(float2 position) { m_position = position; }

void Transformation2D::setScale(float2 scale) { m_scale = scale; }

void Transformation2D::setScale(float scale) { setScale(float2(1, 1) * scale); }

void Transformation2D::setRotation(float rotation) { m_rotation = rotation; }

void Transformation2D::move(float2 movement) { m_position += movement; }

void Transformation2D::rotate(float rotate) { m_rotation += rotate; }

Transformation2D Transformation2D::transform(
	const Transformation2D& target, const Transformation2D& matrix) {
	Transformation2D t = target;
	t.rotate(matrix.getRotation());
	t.setScale(t.getScale() * matrix.getScale());
	t.setPosition(transform(t.getPosition(), matrix));

	return t;
}

float2 Transformation2D::transform(const float2& target, const Transformation2D& matrix) {
	return float2::Transform(target, matrix.getMatrix());
}

void Transformation2D::imgui_properties() {
	ImGui::InputFloat2("Position", (float*)&m_position);
	ImGui::InputFloat2("Scale", (float*)&m_scale);
	ImGui::SliderFloat("Rotation", &m_rotation, 0, XM_PI * 2);
}

Projectile2D::Projectile2D(float2 position, float mass) : Transformation2D(position) { setMass(mass); }

float2 Projectile2D::getVelocity() const { return m_velocity; }

float Projectile2D::getRotationVelocity() const { return m_rotationVelocity; }

float2 Projectile2D::getAcceleration() const { return m_acceleration; }

float2 Projectile2D::getGravity() const { return m_gravity; }

float Projectile2D::getFriction() const { return m_friction; }

float Projectile2D::getMass() const { return m_mass; }

void Projectile2D::setFriction(float friction) { m_friction = friction; }

void Projectile2D::setAcceleration(float2 acceleration) { m_acceleration = acceleration; }

void Projectile2D::setGravity(float2 gravity) { m_gravity = gravity; }

void Projectile2D::setVelocity(float2 velocity) { m_velocity = velocity; }

void Projectile2D::setRotationVelocity(float rotVelocity) { m_rotationVelocity = rotVelocity; }

void Projectile2D::setMass(float mass) { m_mass = mass; }

void Projectile2D::applyForce(float2 force) { m_acceleration += force / m_mass; }

void Projectile2D::update(float dt) {
	// position, velocity, acceleration
	m_velocity += (m_acceleration + m_gravity) * dt;
	m_position += m_velocity * dt;
	m_velocity *= pow(m_friction, dt);
	m_acceleration = float2(0.);

	// rotation
	m_rotation += m_rotationVelocity * dt;
}

void BoundingBox2D::set(const Matrix& matrix, float2 size, float2 alignment) {
	m_matrix = matrix;
	m_size = size;
	m_alignment = alignment;
}

float2 BoundingBox2D::getCenter() const {
	float2 halfSize = m_size / 2.f;
	float2 point_origin = halfSize * m_alignment;
	return float2::Transform(point_origin, m_matrix);
}

vector<float2> BoundingBox2D::getPoints() const { 
	vector<float2> points;
	points.reserve(4);
	float2 halfSize = m_size / 2.f;
	float2 point_origin = halfSize * m_alignment;
	points.push_back(float2::Transform(-point_origin + halfSize * float2(-1, -1), m_matrix));
	points.push_back(float2::Transform(-point_origin + halfSize * float2(1, -1), m_matrix));
	points.push_back(float2::Transform(-point_origin + halfSize * float2(-1, 1), m_matrix));
	points.push_back(float2::Transform(-point_origin + halfSize * float2(1, 1), m_matrix));
	return points;
}

bool BoundingBox2D::isInside(float2 point) const {
	float2 localPoint = float2::Transform(point, m_matrix.Invert());
	float2 halfSize = m_size / 2.f;
	float2 point_origin = halfSize * m_alignment;
	float2 point_start = -point_origin - halfSize;
	float2 point_end = -point_origin + halfSize;
	return (localPoint.x > point_start.x && localPoint.y > point_start.y &&
			localPoint.x < point_end.x && localPoint.y < point_end.y);
}
