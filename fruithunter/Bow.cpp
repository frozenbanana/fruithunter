#include "Bow.h"

Bow::Bow() {
	m_bow.loadAnimated("Bow", 3);
	m_bow.setScale(0.05f);

	m_arrow.load("Arrow");
	m_arrow.setScale(0.05f);
}

Bow::~Bow() {}

void Bow::update(float dt, float3 playerPos, float3 playerForward) {
	m_bow.setPosition(playerPos + playerForward * 0.25f);
	m_bow.updateAnimated(dt);

	if (m_shooting) {
		m_arrow.setPosition(
			m_arrow.getPosition() + m_arrowDirection * dt - float3(0.0f, 0.1 * dt, 0.0f));
		if (m_arrow.getPosition().y < 0.0f) {
			m_shooting = false;
		}
	}
	else {
		m_arrow.setPosition(m_bow.getPosition());
		m_arrow.setRotation(m_bow.getRotation() + float3(0.0f, XM_PI / 2.0f, 0.0f));
	}
}

void Bow::draw(float3 playerRight) {
	m_bow.setPosition(m_bow.getPosition() + playerRight * 0.1f);

	m_bow.draw_animate();
	m_arrow.draw();
}

void Bow::rotate(float angleUp, float angleSide, float3 rotationAxis) {
	float rotX = angleUp * rotationAxis.x;
	float rotZ = angleUp * rotationAxis.z;
	m_bow.setRotation(float3(rotX, angleSide + XM_PI / 2.0f, rotZ));
	// ErrorLogger::log(to_string(rotX) + " " + to_string(rotZ));
}

void Bow::aim() {
	if (!m_shooting) {
		m_aiming = true;
		ErrorLogger::log("Aiming!");
	}
}

void Bow::shoot(float3 direction) {
	if (m_aiming) {
		ErrorLogger::log("SHOOT!");
		m_aiming = false;
		m_shooting = true;
		m_arrowDirection = direction;
	}
}
