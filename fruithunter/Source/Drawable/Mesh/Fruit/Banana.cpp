#include "Banana.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "SceneManager.h"

Banana::Banana(float3 pos) : Fruit(FruitType::BANANA, pos) {
	loadAnimated("Banana", 3);
	setCollisionDataOBB();

	setScale(2.f);

	// no friction (difficult jump calculation if there are)
	m_groundFriction = 60;
	m_airFriction = 60;
}

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
		//float3 adjacentPointTilt = adjacentPointFlat;
		//adjacentPointTilt.y =
		//	terrain->getHeightFromPosition(adjacentPointTilt.x, adjacentPointTilt.z);
		//float3 normal =
		//	Normalize(Normalize((adjacentPointFlat - target).Cross(adjacentPointTilt - target))
		//				  .Cross(adjacentPointFlat - target));
		//bool adjValid = true;
		//if (abs(normal.Dot(float3(0, 1, 0))) < 0.7f)
		//	return false; // too much tilt (unsecure)
		if (!terrain->validPosition(adjacentPointFlat))
			return false; // invalid adjacent point (unsecure)
	}
	return true;
}

void Banana::updateAnimated(float dt) {
	m_meshAnim.setFrameTargets(0, 1);
	m_meshAnim.updateSpecific(0);
}

void Banana::update() {
	Scene* scene = SceneManager::getScene();
	float dt = scene->getDeltaTime();

	checkOnGroundStatus(); // checks if on ground
	updateAnimated(dt);	   // animation stuff
	updateVelocity(dt);	   // update velocity (slowdown and apply accelration)
	behavior();
	updateRespawn();
	move(dt);			  // update position from velocity
	enforceOverTerrain(); // force fruit above ground

	if (getPosition().y < 1)
		respawn();
}

void Banana::_onDeath(Skillshot skillshot) { spawnCollectionPoint(skillshot); }

void Banana::behavior() {
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

					// avoid player
					const float playerRadiusAvoid = 5;
					float distanceToPlayer =
						(SceneManager::getScene()->m_player->getPosition() - target).Length();
					if (distanceToPlayer < playerRadiusAvoid) {
						float withinPlayerFactor = 1 - (distanceToPlayer / playerRadiusAvoid);
						score -= withinPlayerFactor * 20;
					}

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
				// no valid samples
				m_velocity = vertVel; // jump up (fruit might be stuck)
			}
		}
		else {
			// roof
			// bounce off roof
			m_velocity = float3::Reflect(m_velocity, intersection_normal);
		}
	}
}


void Banana::playSound_bounce() {
	float3 cameraPosition = SceneManager::getScene()->m_camera.getPosition();
	SoundID sid = AudioController::getInstance()->play("jump1");
	AudioController::getInstance()->scaleVolumeByDistance(
		sid, (cameraPosition - getPosition()).Length(), 0.2f, 25.f);
	AudioController::getInstance()->setPitch(sid, RandomFloat(-1.f, 1.f) * 0.5f);
}
