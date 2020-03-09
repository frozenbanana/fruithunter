#include "Dragonfruit.h"

Dragonfruit::Dragonfruit(float3 pos) {}

void Dragonfruit::waveFlight(float3 playerPosition, float radius) {


	int degrees = 10 % 360;
	float radians = degrees * XM_PI / 180.f;
	float flightPattern = sin(radians);
}

void Dragonfruit::circulateVertical(float3 playerPosition, float radius) {
	float3 toMelon = m_position - playerPosition;
	toMelon.y = playerPosition.y;
	float angle = XM_PI / 8;
	Matrix rotate = Matrix(cos(angle), 0.f, -sin(angle), 0.f, 0.f, 1.f, 0.f, 0.f, sin(angle), 0.f,
		cos(angle), 0.f, 0.f, 0.f, 0.f, 1.f);


	float3 target = target.Transform(toMelon, rotate);
	target.Normalize();
	target *= 10.f;

	target += playerPosition;
	target.y = playerPosition.y;
}

void Dragonfruit::circulate(float3 playerDir, float radius) {}

void Dragonfruit::behaviorPassive(float3 playerPosition) {
	// Perch on ground or on tree?
	if (withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(ACTIVE);
		return;
	}
	if (m_onGround) {
		circulate(playerPosition, 0.f);
	}
	else {
		m_direction = m_worldHome - m_position;
		if (m_velocity.y < 0.f && m_position.y < m_flapWings) {
			float3 target = m_direction;
			target.Normalize();
			target.y = 1.f;
			jump(target, m_wingStrength);
		}
	}

	m_speed = m_passive_speed;
}

void Dragonfruit::behaviorActive(float3 playerPosition) {
	// when player is near, take flight

	// circulate player in air.
}

void Dragonfruit::behaviorCaught(float3 playerPosition) {
	// just go to player
}
