#include "Melon.h"

Melon::Melon(float3 pos) : Fruit(pos) {
	loadAnimated("Melon", 1);
	m_nrOfFramePhases = 1;
	m_meshAnim.setFrameTargets(0, 0);
}

void Melon::updateAnimated(float dt) {}
