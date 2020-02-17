#include "Melon.h"
//
// void Melon::updatePhases(Phase phases[]) {
//	for (size_t i = 0; i < 3; ++i) {
//		phases[i].position = m_startPos;
//	}
//	for (size_t i = 3; i < 6; ++i) {
//		phases[i].position = m_destinationPos;
//	}
//}

void Melon::roll(float dt) { rotateX(dt * m_rollSpeed); }

Melon::Melon(float3 pos) : Fruit(pos) {
	loadAnimated("Melon", 1);
	m_nrOfFramePhases = 6;
	m_meshAnim.setFrameTargets(0, 0);
	m_rollSpeed = 5;

	setCollisionData(getPosition(), m_meshAnim.getBoundingBoxHalfSizes().y);
}

void Melon::updateAnimated(float dt) {
	m_frameTime += dt;
	if (m_frameTime > 2) {
		m_frameTime = 0.f;
		setDestination();
		lookTo(m_destinationPos);
	}
	float3 tempDir(m_destinationPos - getPosition());
	tempDir.Normalize();
	move(tempDir * dt * 4);
	roll(dt);
}

void Melon::setRollSpeed(float rollSpeed) { m_rollSpeed = rollSpeed; }
