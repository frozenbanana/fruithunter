#include "Bow.h"

Bow::Bow() {
	m_bow.loadAnimated("Bow", 3);
	m_bow.setScale(0.2f);

	m_arrow.load("Arrow");
	m_arrow.setScale(float3(0.2f, 0.2f, 0.5f));
}

Bow::~Bow() {}

void Bow::update(float dt, float3 playerPos, float3 playerForward, float3 playerRight) {
	m_bow.setPosition(
		playerPos + playerForward * 1.0f + playerRight * 0.5f * (1.0f - m_aimMovement));

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

	if (m_shooting) {
		m_arrow.setPosition(
			m_arrow.getPosition() + m_arrowDirection * 10.0f * dt - float3(0.0f, dt, 0.0f));

		if (m_arrow.getPosition().y < 0.0f) {
			m_shooting = false;
		}
	}
	else {
		if (m_charging) {
			m_arrow.setPosition(
				m_bow.getPosition() + playerForward * 0.3f * (1.0f - 1.7f * m_drawFactor));
		}
		else {
			m_arrow.setPosition(m_bow.getPosition() + playerForward * 0.3f);
		}
		m_arrow.setRotation(m_bow.getRotation());
	}

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

void Bow::rotate(float pitch, float yaw) {
	m_bow.setRotation(float3(pitch, yaw, 0.0f));
	// ErrorLogger::log(to_string(rotX) + " " + to_string(rotZ));
}

void Bow::aim() { m_aiming = true; }

void Bow::release() {
	ErrorLogger::log("Release");
	m_charging = false;
	m_chargeReset = false;
}

void Bow::charge() {
	if (!m_shooting && m_chargeReset)
		m_charging = true;
}

void Bow::shoot(float3 direction) {
	m_chargeReset = true;

	if (m_charging) {
		m_charging = false;
		m_shooting = true;
		m_arrowDirection = direction;
	}
}
