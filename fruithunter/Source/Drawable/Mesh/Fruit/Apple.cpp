#include "Apple.h"
#include "Input.h"

Apple::Apple(float3 pos) : Fruit(pos) {
	loadAnimated("Bouncing_apple", 3);
	m_nrOfFramePhases = 6;
	setScale(0.5);
	changeState(AI::State::PASSIVE);
}

void Apple::updateAnimated(float dt) {
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes
	float3 posOrder[6] = {
		m_startPos, m_startPos, m_startPos, m_startPos, m_startPos, m_startPos,
		/*m_heightPos,
		m_destinationPos,
		m_destinationPos,*/
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
			setDestination();
			justChanged = true;
			setRotation(float3(0.f, findRequiredRotation(m_nextDestinationPos), 0.f));
		}

		m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
			frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)]);
	}
	if (m_currentFramePhase >= 2 || justChanged) {
		posOrder[0] = getPosition();
		posOrder[1] = getPosition();
	}
	// Set position
	float3 pos = XMVectorLerp(posOrder[m_currentFramePhase],
		posOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)], m_frameTime);
	setPosition(pos);

	// Update mesh specificly with our frametime
	m_meshAnim.updateSpecific(m_frameTime);
}

void Apple::update(float dt, float3 playerPosition, float height) {
	updateAnimated(dt);
	float3 playerDir = float3(playerPosition - m_startPos);
	playerDir.Normalize();
	float distanceToPlayer = (playerPosition - m_startPos).Length();
	switch (m_currentState) {
	case INACTIVE:
		changeState(PASSIVE);
		break;
	case PASSIVE:
		//------------------------------//
		ErrorLogger::log("I am at " + std::to_string(getPosition().x) + " " +
						 std::to_string(getPosition().y) + " " + std::to_string(getPosition().z));
		if (distanceToPlayer < 5.f) {
			ErrorLogger::log("Inside apple radius");
			AI::changeState(AI::ACTIVE);
		}
		else {
			float x = (float)(rand() % 10);
			float z = (float)(rand() % 10);
			float3 nextDest = float3(x, 0.0, z);
			setNextDestination(nextDest);
		}
		//-----------------------------//
		break;
	case ACTIVE:
		//-----------------------------//
		flee(playerPosition);
		if (distanceToPlayer > 10.f) {
			changeState(PASSIVE);
		}
		//-----------------------------//
		break;
	case CAUGHT:
		//-----------------------------//
		flee(playerPosition);
		//----------------------------//
		break;
	}

	/*else {
		AI::changeState(AI::PASSIVE);
		float x = 5.0f;
		float z = 5.0f;
		float y = 0.0f;
		float3 appleDestination = float3(x, y, z);

		setNextDestination(appleDestination);
	}*/
}

void Apple::flee(float3 playerDir) {
	ErrorLogger::log("Fleeing!");
	float3 start = float3(m_startPos.x, 0.0, m_startPos.z);
	float3 end = float3(playerDir.x, 0.0f, playerDir.z);
	pathfinding(start, end);
	setNextDestination(m_availablePath.front());
}
