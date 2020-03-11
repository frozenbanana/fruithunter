#include "DragonFruit.h"

DragonFruit::DragonFruit() {
	loadAnimated("Dragon", 3);
	vector<string> names{ "Dragon1.mtl", "Dragon2.mtl", "Dragon3.mtl" };
	loadMaterials(names, 3);

	m_nrOfFramePhases = 2;

	changeState(AI::State::PASSIVE);

	m_fruitType = DRAGON;

	// TEMP TAKEN FROM APPLE
	m_activeRadius = 8.f;
	m_passiveRadius = 12.f;

	m_passive_speed = 3.f;
	m_active_speed = 10.f;
	m_caught_speed = 5.f;

	setCollisionDataOBB();
}

DragonFruit::~DragonFruit() {}

void DragonFruit::updateAnimated(float dt) {
	m_frameTime += dt * 4;
	m_frameTime = fmodf(m_frameTime, 4.f);

	if (m_frameTime < 1)
		setFrameTargets(0, 1);
	else if (m_frameTime < 2)
		setFrameTargets(1, 2);
	else if (m_frameTime < 3)
		setFrameTargets(2, 1);
	else if (m_frameTime < 4)
		setFrameTargets(1, 0);
	m_meshAnim.updateSpecific(m_frameTime);

	return;
}
