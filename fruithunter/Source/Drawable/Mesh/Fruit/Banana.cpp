#include "Banana.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "SceneManager.h"

#define PASSIVE_JUMP_POWER 15.f
#define ACTIVE_JUMP_POWER 25.f
#define PANIC_JUMP_POWER 40.f



Banana::Banana(float3 pos) : Fruit(FruitType::BANANA, pos) {
	loadAnimated("Banana", 3);
	m_nrOfFramePhases = 5;
	setFrameTargets(0, 1);

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

	// no friction (difficult jump calculation if there are)
	m_groundFriction = 60;
	m_airFriction = 60;
}

void Banana::behaviorPassive() {
	float dt = SceneManager::getScene()->getDeltaTime();
	float3 playerPosition = SceneManager::getScene()->m_player->getPosition();

	// collision
	float3 ray_point = getBoundingBoxPos() - float3(0, 1, 0) * getHalfSizes();
	float3 ray_distance = m_velocity * dt;
	float3 intersection_position, intersection_normal;
	if (rayCastWorld(ray_point, ray_distance, intersection_position, intersection_normal)) {
		playSound_bounce(); // plays bounce sound

		float3 up(0, 1, 0);
		if (intersection_normal.Dot(up) > 0.5f) {
			// floor
			// jump to new location
			float jumpVelHeight = RandomFloat(7.5f, 12);
			m_velocity = float3(0, jumpVelHeight, 0);

			float3 vertVel = up.Dot(m_velocity) * up;
			float nextApproxCollision = -2 * vertVel.y / m_gravity.y;

			float v = vertVel.y, a = m_gravity.y, p0 = getPosition().y, p1 = -1;
			float maximumJumpHeight = -(v / a) * v;

			// find point samples to jump to
			size_t samples = 16;
			vector<float3> samples_directions;
			samples_directions.reserve(samples);
			vector<float> samples_nextApproxCollision;
			samples_nextApproxCollision.reserve(samples);
			Environment* terrain = m_boundTerrain;
			for (size_t i = 0; i < samples; i++) {
				float r = RandomFloat(0, XM_PI * 2);
				float horLength = (1 - (float)pow(RandomFloat(), 2)) * jumpVelHeight;
				float3 direction = float3(cos(r), 0, sin(r)) * horLength;
				float3 check_point = getPosition() + direction;
				check_point.y = terrain->getHeightFromPosition(check_point.x, check_point.z);

				p1 = check_point.y;
				float sqrtPart = pow(v / a, 2) - 2 * (p0 - p1) / a;
				if (sqrtPart < 0)
					continue; // impossible to reach (too high)
				float approxCollision1 = -v / a + sqrt(sqrtPart);
				float approxCollision2 = -v / a - sqrt(sqrtPart);
				float approxCollision = max(approxCollision1, approxCollision2);
				// append if valid
				if (validAndSecureJumpTarget(getPosition(), check_point, maximumJumpHeight)) {
					samples_directions.push_back(direction);
					samples_nextApproxCollision.push_back(approxCollision);
				}
			}
			if (samples_directions.size() > 0) {
				// calculate scores and pick best
				int targetIndex = -1;
				float bestScore = 0;
				for (size_t i = 0; i < samples_directions.size(); i++) {
					float score = 0;
					float3 target = getPosition() + samples_directions[i];
					target.y = terrain->getHeightFromPosition(target.x, target.z);

					// jump long distances (problem: high risk of flying away on collision)
					// float horDistFactor = (samples_directions[i].Length() - 1) / jumpVelHeight;
					// score += horDistFactor * 10; // gain score for further jumps

					// avoid player
					const float playerRadiusAvoid = 5;
					float distanceToPlayer =
						(SceneManager::getScene()->m_player->getPosition() - target).Length();
					if (distanceToPlayer < playerRadiusAvoid) {
						float withinPlayerFactor = 1 - (distanceToPlayer / playerRadiusAvoid);
						score -= withinPlayerFactor * 20;
					}

					// traverse height (problem: they stack on high points)
					// float heightMargin = 0.7f;
					// float heightFactor =
					//	(target.y - getPosition().y) / (maximumJumpHeight * heightMargin);
					// score += heightFactor * 10;

					// keep best
					if (i == 0 || score > bestScore) {
						bestScore = score;
						targetIndex = i;
					}
				}
				// set velocity to go to new point
				m_velocity = vertVel + samples_directions[targetIndex] /
										   samples_nextApproxCollision[targetIndex];
			}
			else {
				// no valid positions
				if (getPosition().y > 1) {
					m_velocity = vertVel; // jump up (fruit probably stuck)
				}
				else {
					// below sea level and cant jump anywhere
					m_velocity = vertVel;
					respawn();
				}
			}
		}
		else {
			// roof
			// bounce off roof
			m_velocity = float3::Reflect(m_velocity, intersection_normal);
		}
	}
}

void Banana::behaviorActive() {}

void Banana::behaviorCaught() {}

bool Banana::validAndSecureJumpTarget(float3 source, float3 target, float maximumJumpHeight) {
	Environment* terrain = m_boundTerrain;
	// check if valid point
	if (!terrain->validPosition(target))
		return false;
	// only jump if have height marginal (might hit wall before floor if too high)
	float heightMargin = 0.7f;
	if (target.y - source.y > maximumJumpHeight * heightMargin)
		return false;
	// check if secure
	const float secureDistance = 0.25f;
	for (size_t i = 0; i < 4; i++) {
		float r = ((float)i / 4) * XM_PI * 2;
		float3 adjacentPointFlat = target + float3(cos(r), 0, sin(r) * secureDistance);
		float3 adjacentPointTilt = adjacentPointFlat;
		adjacentPointTilt.y =
			terrain->getHeightFromPosition(adjacentPointTilt.x, adjacentPointTilt.z);
		float3 normal =
			Normalize(Normalize((adjacentPointFlat - target).Cross(adjacentPointTilt - target))
						  .Cross(adjacentPointFlat - target));
		bool adjValid = true;
		if (abs(normal.Dot(float3(0, 1, 0))) < 0.7f)
			return false; // too much tilt (unsecure)
		if (!terrain->validPosition(adjacentPointFlat))
			return false; // invalid adjacent point (unsecure)
	}
	return true;
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

void Banana::_onDeath(Skillshot skillshot) { spawnCollectionPoint(skillshot); }


void Banana::playSound_bounce() {
	float3 cameraPosition = SceneManager::getScene()->m_camera.getPosition();
	SoundID sid = AudioController::getInstance()->play("jump1");
	AudioController::getInstance()->scaleVolumeByDistance(
		sid, (cameraPosition - getPosition()).Length(), 0.2f, 25.f);
	AudioController::getInstance()->setPitch(sid, RandomFloat(-1.f, 1.f) * 0.5f);
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
