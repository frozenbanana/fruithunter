#include "Pomegranate.h"
#include "Input.h"
#include "SceneManager.h"

void Pomegranate::behavior() {
	Scene* scene = SceneManager::getScene();
	float3 playerPosition = scene->m_player->getPosition();
	float dt = SceneManager::getScene()->getDeltaTime();

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

				float3 target = findJumpLocation(3, 3, 16, 0);
				jumpToLocation(target, float2(0.5f, 0.75f));
				float3 flatVel = m_velocity * float3(1, 0, 1);
				if (flatVel.Length() > 0)
					lookTo(flatVel);
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

void Pomegranate::update() {
	Scene* scene = SceneManager::getScene();
	float dt = scene->getDeltaTime();

	checkOnGroundStatus(); // checks if on ground
	behavior();
	updateAnimated(dt); // animation stuff
	updateVelocity(dt);
	move(dt);
	updateRespawn();
	enforceOverTerrain(); // force fruit above ground

	// respawn
	if (getPosition().y <= 1)
		respawn();
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
	setCollisionDataOBB();

	setScale(m_baseScale);

	m_groundFriction = 60.f;
	m_airFriction = 60.f;
}
