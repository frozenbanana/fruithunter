#include "Apple.h"
#include "Input.h"

Apple::Apple(float3 pos) : Fruit(pos) {
	loadAnimated("Apple", 3);
	vector<string> names{ "Apple.mtl", "Apple2.mtl", "Apple3.mtl" };
	loadMaterials(names, 3);

	m_nrOfFramePhases = 6;
	setScale(0.5);
	changeState(AI::State::PASSIVE);

	m_fruitType = APPLE;

	m_activeRadius = 4.f;
	m_passiveRadius = 8.f;

	m_passive_speed = 3.f;
	m_active_speed = 10.f;
	m_caught_speed = 5.f;

	setCollisionDataOBB();
}

void Apple::behaviorPassive(float3 playerPosition) {
	float terrainHeight = TerrainManager::getInstance()->getHeightFromPosition(m_position);
	// Check if not at home
	if (withinDistanceTo(playerPosition, m_activeRadius)) {
		stopMovement();
		changeState(ACTIVE);
	}
	else {
		if (!withinDistanceTo(m_worldHome, ARRIVAL_RADIUS) && m_onGround) {
			// Check if there is no other path on going
			if (m_availablePath.empty()) {
				if (m_nrOfTriesGoHome++ < 10) {
					makeReadyForPath(m_worldHome); // go home
				}
				else {
					setWorldHome(m_position + float3(0.001f, 0, 0.001f));
					m_nrOfTriesGoHome = 0;
				}
				m_speed = m_passive_speed;
			}
			else {
				float3 jumpTo = (m_availablePath.back() - m_position);
				jumpTo.Normalize();
				jumpTo.y = 1.f;
				jump(jumpTo, 1.f);
			}
		}
		else { // Just jump when home
			if (m_onGround) {
				m_speed = 0.f;
				jump(float3(0.0f, 1.0f, 0.0), 20.f);
				m_nrOfJumps++;
				if (m_nrOfJumps >= MAXNROFJUMPS) {
					float3 newHome = m_worldHome;
					newHome += float3(RandomFloat(-10.f, 10.f), 0.f, RandomFloat(-10.f, 10.f));
					newHome.y = TerrainManager::getInstance()->getHeightFromPosition(newHome);
					if (isValid(newHome, m_position)) {
						m_worldHome = newHome;
						m_nrOfJumps = 0;
					}
				}
			}
		}
	}
}

void Apple::behaviorActive(float3 playerPosition) {
	if (!withinDistanceTo(playerPosition, m_passiveRadius)) {
		changeState(PASSIVE);
		stopMovement();
	}
	else {
		flee(playerPosition);
		m_speed = m_active_speed;
		m_readyForPath = true;
	}
}

void Apple::behaviorCaught(float3 playerPosition) {
	if (atOrUnder(TerrainManager::getInstance()->getHeightFromPosition(m_position))) {
		makeReadyForPath(playerPosition);
		m_speed = m_caught_speed;
	}
	lookTo(playerPosition);
}

void Apple::updateAnimated(float dt) {
	m_startAnimationPosition = m_position;
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes
	float3 posOrder[6] = {
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
	};
	bool justChanged = false;
	float frameSpeedOrder[] = { 4.f, 5.f, 2.0f, 1.9f, 4.f, 2.f };
	m_frameTime += dt * frameSpeedOrder[m_currentFramePhase];
	if (Input::getInstance()->keyDown(Keyboard::K))
		int th = 0;
	// Maybe change keyframes
	if (m_frameTime > 1) {
		m_frameTime -= 1;
		m_currentFramePhase = m_currentFramePhase + 1;

		if (m_currentFramePhase == m_nrOfFramePhases) {
			m_currentFramePhase = 0;
			setAnimationDestination();
			justChanged = true;
			lookTo(m_nextDestinationAnimationPosition);
		}

		m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
			frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)]);
	}
	if (m_currentFramePhase >= 2 || justChanged) {
		posOrder[0] = getPosition();
		posOrder[1] = getPosition();
	}

	// Update mesh specificly with our frametime
	m_meshAnim.updateSpecific(m_frameTime);



	if (Input::getInstance()->keyPressed(Keyboard::O)) {
		m_currentMaterial = 0;
	}
	if (Input::getInstance()->keyPressed(Keyboard::P)) {
		m_currentMaterial = 1;
	}
}

void Apple::flee(float3 playerPos) {
	// Update fleeing path if ther is none
	if (m_availablePath.empty()) {
		float3 runTo = m_position - playerPos;
		runTo.Normalize();
		runTo *= m_passiveRadius;
		runTo += m_position;
		m_destination = runTo;
		m_readyForPath = true;
	}
	// set new velocity from path
}
