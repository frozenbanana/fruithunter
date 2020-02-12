#include "Apple.h"
#include "Input.h"

Apple::Apple(float3 pos) : Fruit(pos) {
	loadAnimated("Bouncing_apple", 3);
	m_nrOfFramePhases = 6;
	setScale(0.5);
	changeState(AI::State::PASSIVE);
	m_direction = float3((float)(rand() % 1), 0.0f, (float)(rand() % 1));
	m_velocity = float3(0.f);
	m_direction.Normalize();
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

void Apple::move(float dt) {
	m_position += m_velocity * dt;
	setPosition(m_position);
}

void Apple::update(float dt, float3 playerPosition, float height) {
	float gravity = 9.82f * dt * dt / 2.;

	if (m_inAir) {
		m_velocity.y += -gravity;
	}

	if (m_position.y < height) {
		m_position.y = height;
		m_velocity.y = 0.f;
		m_inAir = false;
	}


	// updateAnimated(dt);
	float3 playerDir = float3(playerPosition - m_position);
	playerDir.Normalize();
	float distanceToPlayer = (playerPosition - m_position).Length();
	ErrorLogger::log("dist" + std::to_string(distanceToPlayer) + ", velo: " +
					 std::to_string(m_velocity.y) + ", inAir: " + std::to_string(m_inAir));
	if (distanceToPlayer < 5.f && !m_inAir) {
		ErrorLogger::log("Trying to jump");
		m_velocity.y += 20000.0 * dt * dt / 2.f;
		m_inAir = true;
	}
	move(dt);

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
