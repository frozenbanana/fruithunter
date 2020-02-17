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
	m_fruitType = MELON;

	setCollisionData(getPosition(),
		m_meshAnim.getBoundingBoxHalfSizes().y); // sets to sphere with radius same as y halfsize;
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

void Melon::update(float dt, float3 playerPos) { 
	setNextDestination(playerPos);
}
