#include "Banana.h"

void Banana::behaviorPassive(float3 playerPosition) {
	m_bounceDestination *= 0.01;
	ErrorLogger::log("Banana:: Doing Passive.");
	if ((m_position - playerPosition).Length() < 3.f) {
		changeState(ACTIVE);
	}
}
void Banana::behaviorActive(float3 playerPosition) {
	m_bounceDestination *= 10;
	ErrorLogger::log("Banana:: Doing active.");
	if ((m_position - playerPosition).Length() > 10.f) {
		changeState(PASSIVE);
	}
}
void Banana::behaviorCaught(float3 playerPosition) { ErrorLogger::log("Banana:: Doing caught."); }

Banana::Banana(float3 pos) : Fruit(pos) {
	loadAnimated("Banana", 3);
	m_nrOfFramePhases = 5;
	m_maxBounciness = 3;
	m_bounciness = 0;
	m_state = Jump;
	rotRandom();
	setScale(2.f);
	m_currentState = PASSIVE;
}

void Banana::updateAnimated(float dt) {
	switch (m_state) {
	case Jump:
		updateFirstJump(dt);
		break;
	case Bounce:
		updateBounce(dt);
		break;
	case Stopped:
		updateStopped(dt);
		break;
	default:
		ErrorLogger::log("wrong state in banana");
	}
}

void Banana::update(float dt, Vector3 playerPosition, TerrainManager* terrain) {

	m_bounceDestination = terrain->getNormalFromPosition(getPosition());
	//m_bounceDestination.y = 0;
	m_bounceDestination.Normalize();
	
	m_bounceDestination += getPosition();
	m_bounceDestination.y = terrain->getHeightFromPosition(m_bounceDestination);
	setNextDestination(m_bounceDestination);
	updateAnimated(dt);


	doBehavior(playerPosition);
}

void Banana::updateFirstJump(float dt) {
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes
	float3 posOrder[6] = {
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_heightAnimationPosition,
		m_destinationAnimationPosition,
		m_destinationAnimationPosition,
	};
	bool justChanged = false;
	float frameSpeedOrder[] = { 4.f, 5.f, 2.0f, 1.9f, 4.f, 2.f };
	m_frameTime += dt * frameSpeedOrder[m_currentFramePhase];
	// Maybe change keyframes
	if (m_frameTime > 1) {
		m_frameTime -= 1;
		m_currentFramePhase = m_currentFramePhase + 1;

		if (m_currentFramePhase == m_nrOfFramePhases) {
			m_currentFramePhase = 0;
			justChanged = true;
			setRotation(float3(0.f, findRequiredRotation(m_nextDestinationAnimationPosition), 0.f));

			// start bouncing
			m_nrOfFramePhases = 3;
			m_bounciness = m_maxBounciness;
			m_state = Bounce;
			bounce();
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

void Banana::updateBounce(float dt) {
	// TODO: Bounce. physics?

	int frameOrder[] = { 1, 2, 1 }; // Order of using keyframes
	float3 posOrder[] = {
		m_startAnimationPosition,
		m_heightAnimationPosition,
		m_destinationAnimationPosition,
	};
	bool justChanged = false;
	float frameSpeedOrder[] = { 2.f, 2.0f, 15.9f };
	m_frameTime += dt * frameSpeedOrder[m_currentFramePhase];
	// Maybe change keyframes
	if (m_frameTime > 1) {
		m_frameTime -= 1;
		m_currentFramePhase = m_currentFramePhase + 1;

		if (m_currentFramePhase == m_nrOfFramePhases) {
			m_currentFramePhase = 0;
			justChanged = true;
			bounce();
			setRotation(float3(0.f, findRequiredRotation(m_nextDestinationAnimationPosition), 0.f));
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
	rotate(m_rotation * dt);
	// Update mesh specificly with our frametime
	m_meshAnim.updateSpecific(m_frameTime);
}

void Banana::updateStopped(float dt) {
	// TODO: Straighten up and prepare to jump again.
}

void Banana::stop() {}

void Banana::bounce() {
	setDestination();
	if (m_bounciness <= 0) { // Will be stopped
		m_state = Jump;
		m_nrOfFramePhases = 6;
	}
	else {
		m_heightAnimationPosition.y += m_bounciness;
		m_bounciness -= 0.3f;
		rotRandom();
	}
}
