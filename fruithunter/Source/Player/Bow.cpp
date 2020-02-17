#include "Bow.h"

Bow::Bow() {
	m_bow.loadAnimated("Bow", 3);
	m_bow.setScale(0.2f);

	m_arrow.load("Arrow");
	m_arrow.setScale(float3(0.2f, 0.2f, m_arrowLength));
	m_arrow.setCollisionData(m_arrow.getPosition(), m_arrow.getHalfSizes());
}

Bow::~Bow() {}

void Bow::update(float dt, float3 playerPos, float3 playerForward, float3 playerRight) {
	// Set bow position based on player position and direction.
	m_bow.setPosition(
		playerPos + playerForward * m_armLength + playerRight * 0.5f * (1.0f - m_aimMovement));

	// Bow animation.
	if (m_charging) {
		m_drawFactor = min(0.99f, m_drawFactor + dt);
		m_bow.updateAnimatedSpecific(m_drawFactor);
		m_bow.setFrameTargets(0, 1);
	}
	else {
		m_drawFactor = max(0.0f, m_drawFactor - 5.0f * dt);
		m_bow.updateAnimatedSpecific(m_drawFactor);
		m_bow.setFrameTargets(0, 1);
	}

	// Update arrow.
	if (m_shooting) {
		arrowPhysics(
			dt, float3(10.f, 0.f, 0.f)); // Updates arrow in flight, wind is currently hard coded.
		m_arrow.setPosition(m_arrow.getPosition() + m_arrowVelocity * dt);

		if ((m_bow.getPosition() - m_arrow.getPosition()).Length() >
			20.0f) { // replace with collision later
			m_shooting = false;
		}
	}
	else {
		if (m_charging) {
			// Move arrow with bowstring. Hardcoded values determined by experimentation.
			m_arrow.setPosition(
				m_bow.getPosition() + playerForward * 0.3f * (1.0f - 1.6f * m_drawFactor));
		}
		else {
			m_arrow.setPosition(m_bow.getPosition() + playerForward * 0.3f);
		}
		m_arrow.setRotation(m_bow.getRotation());
	}

	// Move bow towards the center while aiming.
	if (m_aiming) {
		if (m_aimMovement < 0.9f)
			m_aimMovement += dt * 4.0f;
	}
	else {
		if (m_aimMovement > 0.0f)
			m_aimMovement -= dt * 4.0f;
	}

	m_aiming = false;
}

void Bow::draw() {
	m_bow.draw_animate();
	m_arrow.draw();
}

void Bow::rotate(float pitch, float yaw) { m_bow.setRotation(float3(pitch, yaw, 0.0f)); }

void Bow::aim() { m_aiming = true; }

void Bow::release() { // Stops charging
	if (m_aimMovement < 0.1f) {
		m_charging = false;
		m_chargeReset = false;
	}
}

void Bow::charge() { // Draws the arrow back on the bow
	if (!m_shooting && m_chargeReset)
		m_charging = true;
}

void Bow::shoot(float3 direction) { // Shoots/fires the arrow
	m_chargeReset = true;

	if (m_charging) {
		m_charging = false;
		m_shooting = true;

		float bowEfficiencyConstant = 400.0f;
		float bowMaterialConstant = 0.05f;

		float velocity = pow((bowEfficiencyConstant * m_drawFactor) /
								 (m_arrowMass + m_bowMass * bowMaterialConstant),
			0.5f);

		direction.Normalize();

		m_arrowVelocity = direction * velocity;
		m_oldArrowVelocity = m_arrowVelocity; // Required to calc rotation
	}
}

void Bow::arrowPhysics(float dt, float3 windVector) { // Updates arrow in flight
	// Update acceleration

	float3 relativeVelocity = m_arrowVelocity - windVector;

	calcArea(relativeVelocity);

	float totalDragTimesLength = -m_arrowArea * relativeVelocity.Length() / m_arrowMass;

	float3 acceleration = float3(totalDragTimesLength * relativeVelocity.x,
		(totalDragTimesLength * relativeVelocity.y) - 9.82f,
		totalDragTimesLength * relativeVelocity.z);

	m_arrowVelocity += acceleration * dt;

	float angle = calcAngle(m_arrowVelocity, m_oldArrowVelocity);
	m_arrow.rotateX(angle);

	m_oldArrowVelocity = m_arrowVelocity;
}

void Bow::calcArea(float3 relativeWindVector) {
	float angle = calcAngle(relativeWindVector, m_arrowVelocity);
	m_arrowArea = ((1 - sin(angle)) * 0.0001f) + (sin(angle) * 0.005f);
}

float Bow::calcAngle(float3 vec1, float3 vec2) {
	float3 normalisedVec1 = vec1;
	normalisedVec1.Normalize();

	float3 normalisedVec2 = vec2;
	normalisedVec2.Normalize();

	return acos(normalisedVec1.Dot(normalisedVec2));
}
