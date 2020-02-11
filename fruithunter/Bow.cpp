#include "Bow.h"

Bow::Bow() {
	m_bow.loadAnimated("Bow", 3);
	m_bow.setScale(0.25f);

	m_arrow.load("Arrow");
	m_arrow.setScale(0.25f);
}

Bow::~Bow() {}

void Bow::update(float dt, float3 playerPos, float3 playerForward, float3 playerRight) {
	m_bow.setPosition(
		playerPos + playerForward * 1.0f + playerRight * 0.5f * (1.0f - m_aimMovement));

	if (m_aiming) {
		if (m_aimMovement < 0.9f)
			m_aimMovement += dt * 2.0f;

		m_arrow.setPosition(m_bow.getPosition());
		m_arrow.setRotation(m_bow.getRotation());
	}
	else if (m_shooting) {
		if (m_aimMovement > 0.0f)
			m_aimMovement -= dt * 4.0f;

		m_arrow.setPosition(
			m_arrow.getPosition() + m_arrowDirection * 10.0f * dt - float3(0.0f, dt, 0.0f));
		if (m_arrow.getPosition().y < 0.0f) {
			m_shooting = false;
		}
	}
	else {
		if (m_aimMovement > 0.0f)
			m_aimMovement -= dt * 4.0f;

		m_arrow.setPosition(m_bow.getPosition());
		m_arrow.setRotation(m_bow.getRotation());
	}

	m_aiming = false;
	m_bow.updateAnimated(dt);
}

void Bow::draw() {
	m_bow.draw_animate();
	m_arrow.draw();
}

void Bow::rotate(float pitch, float yaw) {
	m_bow.setRotation(float3(pitch, yaw, 0.0f));
	// ErrorLogger::log(to_string(rotX) + " " + to_string(rotZ));
}

void Bow::aim() {
	if (!m_shooting) {
		m_aiming = true;
	}
}

void Bow::charge() { m_charging = true; }

void Bow::shoot(float3 direction) {
	if (m_charging) {
		m_charging = false;
		m_shooting = true;
		m_arrowDirection = direction;
	}
}
