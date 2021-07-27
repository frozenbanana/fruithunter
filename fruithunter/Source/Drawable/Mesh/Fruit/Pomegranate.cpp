#include "Pomegranate.h"
#include "Input.h"
#include "SceneManager.h"

void Pomegranate::behavior(float dt) {
	Scene* scene = SceneManager::getScene();
	float3 playerPosition = scene->m_player->getPosition();

	if (m_ignited) {
		m_igniteTimer = Clamp(m_igniteTimer + dt, 0.f, 1.f);

		float iterations = 4;
		float scale_min = m_baseScale * 0.675f, scale_max = m_baseScale;
		float factor = cos(m_igniteTimer * 2 * XM_PI * iterations) * 0.5f + 0.5f;
		float factor_scale = lerp(scale_min, scale_max, factor);
		float3 factor_color = lerp(float3(1.f, 0.f, 0.f), float3(1.f), factor);
		setScale(factor_scale);
		m_baseColor = factor_color;

		if (m_igniteTimer >= 1) {
			onDeath(m_hitSkillshot);
		}
	}
	else {
		// states and animation
		if (m_jumping) {
			// in air
			m_jumpAnim = Clamp(m_jumpAnim + dt, 0.f, 1.f);
			m_frameTime = (1.f / 2.2f) + m_jumpAnim;
		}
		else {
			// on ground
			m_charge = Clamp(m_charge + dt, 0.f, 1.f);
			m_frameTime = m_charge / 2.2f;
			if (m_charge >= 1) {
				// jump
				m_charge = 0;
				m_jumping = true;

				jumpToRandomLocation(float2(4.f, 7.f), 12);
				lookTo(m_velocity * float3(1, 0, 1));
				playSound_bounce();
			}
		}
	}

	// collision
	float3 ray_point = getBoundingBoxPos() - float3(0, 1, 0) * getHalfSizes();
	float3 ray_distance = m_velocity * dt;
	float3 intersection_position, intersection_normal;
	if (rayCastWorld(ray_point, ray_distance, intersection_position, intersection_normal)) {
		intersection_normal.Normalize();
		float3 up(0, 1, 0);
		if (intersection_normal.Dot(up) >= 0.7f) {
			m_jumping = false;
			m_jumpAnim = 0;
			// flat
			// bounce on ground
			float reflectFactor = 0.25f;
			m_velocity =
				float3::Reflect(m_velocity, intersection_normal) * reflectFactor; // reflect
		}
		else {
			// steep
			// slide downhill
			m_velocity -= m_velocity.Dot(intersection_normal) *
						  intersection_normal; // remove force against normal
		}
	}
}

bool Pomegranate::validAndSecureJumpTarget(float3 source, float3 target, float maximumJumpHeight) {
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

void Pomegranate::jumpToRandomLocation(float2 heightRange, size_t samples) {
	float jumpVelHeight = RandomFloat(heightRange.x, heightRange.y);
	float3 vertVel = float3(0, jumpVelHeight, 0);
	float nextApproxCollision = -2 * jumpVelHeight / m_gravity.y;

	// max jump height
	float v = jumpVelHeight, a = m_gravity.y, p0 = getPosition().y, p1 = -1;
	float maximumJumpHeight = -(v / a) * v;

	// find point samples to jump to
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
		m_velocity =
			vertVel + samples_directions[targetIndex] / samples_nextApproxCollision[targetIndex];
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

void Pomegranate::update() {
	Scene* scene = SceneManager::getScene();
	float dt = scene->getDeltaTime();

	checkOnGroundStatus(); // checks if on ground
	behavior(dt);
	updateAnimated(dt); // animation stuff
	updateVelocity(dt);

	// float3 movement = m_velocity * dt;
	// float3 nextPosition = getPosition() + movement;
	// float height = SceneManager::getScene()->m_terrains.getHeightFromPosition(nextPosition);
	// if (nextPosition.y < height) {
	//	m_velocity.y = 0;
	//}

	move(dt);
	updateRespawn();
	enforceOverTerrain(); // force fruit above ground
}

void Pomegranate::updateAnimated(float dt) {
	vector<int> frameOrder = { 0, 1, 2, 0 };
	vector<float> frameTime = { 1, 0.2f, 1 };
	float timeSum = 0;
	for (size_t i = 0; i < frameTime.size(); i++)
		timeSum += frameTime[i];

	float frameEdge = 0;
	float anim = m_frameTime * timeSum;
	for (size_t i = 0; i < frameTime.size(); i++) {
		float nextFrameEdge = frameEdge + frameTime[i];
		if (anim < nextFrameEdge) {
			// found frame
			setFrameTargets(frameOrder[i], frameOrder[i + 1]);
			float linFactor = (anim - frameEdge) / (nextFrameEdge - frameEdge);
			float smoothFactor = 0.5f - cos(linFactor * XM_PI) * 0.5f;
			m_meshAnim.updateSpecific(smoothFactor);
			break;
		}
		frameEdge = nextFrameEdge;
	}
}

void Pomegranate::_onDeath(Skillshot skillshot) {
	spawnCollectionPoint(skillshot);

	// spawn explosion
	shared_ptr<EffectSystem> effect = make_shared<EffectSystem>();
	effect->loadFromPreset("pomegranate_explosion_v2");
	effect->setPosition(getPosition());
	effect->setRotation(float3(-XM_PI / 2, 0, 0));
	effect->burst();
	effect->markForDeletion(); // will be deleted when effect has finished
	SceneManager::getScene()->m_effects.push_back(effect);

	// play explosion sound
	float3 cameraPosition = SceneManager::getScene()->m_camera.getPosition();
	SoundID sid = AudioController::getInstance()->play("pomegranate_explosion");
	AudioController::getInstance()->scaleVolumeByDistance(
		sid, (cameraPosition - getPosition()).Length(), 10, 100);
	AudioController::getInstance()->setPitch(sid, 0);

	// throw player
	float2 throwRange = float2(5, 10);
	float throwForce = 25;
	Player* player = SceneManager::getScene()->m_player.get();
	float3 toPlayer = player->getPosition() - getPosition();
	float playerDistance = toPlayer.Length();
	if (playerDistance < throwRange.y) {
		float throwFactor =
			1 - Clamp((playerDistance - throwRange.x) / (throwRange.y - throwRange.x), 0.f, 1.f);
		float force = pow(throwFactor, 2) * throwForce;
		player->applyForce(Normalize(toPlayer) * force);
	}

	// kill surrounding fruit
	float killRange = 10;
	vector<shared_ptr<Fruit>>* fruits = &SceneManager::getScene()->m_fruits;
	for (size_t i = 0; i < fruits->size(); i++) {
		float dist = (fruits->at(i)->getPosition() - getPosition()).Length();
		if (this != fruits->at(i).get() && dist < killRange)
			fruits->at(i)->onHit(Skillshot::SS_BRONZE);
	}
}

void Pomegranate::onHit(Skillshot skillshot) {
	if (!m_ignited) {
		// recover stamina
		Player* player = SceneManager::getScene()->m_player.get();
		player->getStaminaBySkillshot(skillshot); // give stamina

		// ignite
		m_ignited = true;
		m_hitSkillshot = skillshot;
		m_igniteTimer = 0;

		// sound
		float3 cameraPosition = SceneManager::getScene()->m_camera.getPosition();
		SoundID sid = AudioController::getInstance()->play("pomegranate_ignition");
		AudioController::getInstance()->scaleVolumeByDistance(
			sid, (cameraPosition - getPosition()).Length(), 10, 100);
		AudioController::getInstance()->setPitch(sid, 0);
	}
}

void Pomegranate::playSound_bounce() {
	float3 cameraPosition = SceneManager::getScene()->m_camera.getPosition();
	SoundID sid = AudioController::getInstance()->play("jump1");
	AudioController::getInstance()->scaleVolumeByDistance(
		sid, (cameraPosition - getPosition()).Length(), 0.2f, 25.f);
	AudioController::getInstance()->setPitch(sid, RandomFloat(-1.f, -0.25f));
}

Pomegranate::Pomegranate(float3 position) : Fruit(FruitType::POMEGRANATE, position) {
	loadAnimated("pomegranate", 3);
	setFrameTargets(0, 1);

	setCollisionDataOBB();

	setScale(m_baseScale);

	m_speed = 0;
	m_groundFriction = 60.f;
	m_airFriction = 60.f;
}
