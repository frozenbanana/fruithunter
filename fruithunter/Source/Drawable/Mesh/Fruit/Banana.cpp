#include "Banana.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "SceneManager.h"

#define PASSIVE_JUMP_POWER 15.f
#define ACTIVE_JUMP_POWER 25.f
#define PANIC_JUMP_POWER 40.f



Banana::Banana(float3 pos) : Fruit(pos) {
	m_fruitType = BANANA;
	loadAnimated("Banana", 3);

	m_nrOfFramePhases = 5;
	m_maxBounciness = 3;
	m_bounciness = 0;
	m_state = Jump;
	rotRandom();
	setScale(2.f);
	m_currentState = PASSIVE;
	m_worldHome = getPosition();
	setCollisionDataOBB();
	m_speed = 1.f;
	m_activeRadius = 5.f;
	m_passiveRadius = 3.f;

	m_groundFriction = 60;
	m_airFriction = 30;

	setFrameTargets(0, 1);
}

void Banana::behaviorPassive() {
	float dt = SceneManager::getScene()->getDeltaTime();
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	// if below sea level
	if (getPosition().y <= 1.f) {
		// jump towards home
		float3 target = m_worldHome - getPosition();
		target.Normalize();
		target.y = 1.f;
		jump(target, 10.f);
		return;
	}
	
	// collision
	float3 ray_point = getPosition() - float3(0, 1, 0) * getHalfSizes().y;
	float3 ray_distance = m_velocity * dt;
	float3 intersection_position, intersection_normal;
	if (rayCastWorld(ray_point, ray_distance, intersection_position, intersection_normal)) {
		if (withinDistanceTo(playerPosition, m_activeRadius)) {
			// Freakout if player is close
			// Go bananas!
			intersection_normal.x += RandomFloat(-1.f, 1.f);
			intersection_normal.z += RandomFloat(-1.f, 1.f);
			intersection_normal.y = 1.0f;
			jump(intersection_normal, ACTIVE_JUMP_POWER);
			// play audio
			playSound_bounce();
		}
		else {
			// bounce
			bounce(intersection_normal, 1.35f, 5);
			// clamp velocity
			//m_velocity = Normalize(m_velocity) * min(m_velocity.Length(), m_velocitySpeed_max);
		}
	}
}

void Banana::behaviorActive() {}

void Banana::behaviorCaught() {}

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

void Banana::release(float3 direction) {
	// start bouncing
	m_nrOfFramePhases = 3;
	m_bounciness = m_maxBounciness;
	m_state = Bounce;
	changeState(RELEASED);
	m_direction = direction;
	m_velocity = m_direction * THROWVELOCITY;
	m_afterRealease = true;
}


void Banana::playSound_bounce() {
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();
	SoundID sid = AudioController::getInstance()->play("jump1");
	AudioController::getInstance()->scaleVolumeByDistance(
		sid, (playerPosition - getPosition()).Length(), 0.2, 25);
	AudioController::getInstance()->setPitch(sid, RandomFloat(-1, 1) * 0.5);
}

void Banana::bounce(
	float3 normal, float bounceReflectMultiplier, float randomHorizontalMultiplier) {
	// reflect
	float3::Reflect(m_velocity, normal, m_velocity);
	m_velocity *= bounceReflectMultiplier;
	// random direction (greater effect if velocity point up or down)
	float tiltFactor = abs(normal.Dot(float3(0, 1, 0)));
	float3 randomVel =
		tiltFactor * float3(RandomFloat(-1, 1), 0, RandomFloat(-1, 1)) * randomHorizontalMultiplier;
	m_velocity += randomVel;

	// play audio 
	playSound_bounce();
}

void Banana::updateFirstJump(float dt) {
	int frameOrder[] = { 0, 1, 0, 2, 0, 1 }; // Order of using keyframes

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
			lookTo(m_nextDestinationAnimationPosition);

			// start bouncing
			m_nrOfFramePhases = 3;
			m_bounciness = m_maxBounciness;
			m_state = Bounce;
			bounce();
		}

		m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
			frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)]);
	}

	// Update mesh specificly with our frametime
	m_meshAnim.updateSpecific(m_frameTime);
}

void Banana::updateBounce(float dt) {
	// TODO: Bounce. physics?

	int frameOrder[] = { 1, 2, 1 }; // Order of using keyframes
	float3 posOrder[] = {
		m_startAnimationPosition,
		m_startAnimationPosition,
		m_startAnimationPosition,
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
			lookTo(m_nextDestinationAnimationPosition);
		}

		m_meshAnim.setFrameTargets(frameOrder[m_currentFramePhase],
			frameOrder[(m_currentFramePhase + 1) % (m_nrOfFramePhases)]);
	}
	if (m_currentFramePhase >= 2 || justChanged) {
		posOrder[0] = getPosition();
		posOrder[1] = getPosition();
	}

	rotate(m_rotation * dt);

	m_meshAnim.updateSpecific(m_frameTime);
}

void Banana::updateStopped(float dt) {
	// TODO: Straighten up and prepare to jump again.
}

void Banana::bounce() {
	setAnimationDestination();
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
