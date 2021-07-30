#include "Apple.h"
#include "Input.h"
#include "SceneManager.h"
#include "AudioController.h"

Apple::Apple(float3 pos) : Fruit(FruitType::APPLE, pos) {
	loadAnimated("Apple", 3);
	setCollisionDataOBB();

	setScale(0.5);

	m_groundFriction = 60.f; // 5
	m_airFriction = 60.f;
}

void Apple::playSound_bounce() {
	float3 cameraPosition = SceneManager::getScene()->m_camera.getPosition();
	SoundID sid = AudioController::getInstance()->play("jump1");
	AudioController::getInstance()->scaleVolumeByDistance(
		sid, (cameraPosition - getPosition()).Length(), 0.2f, 25.f);
	AudioController::getInstance()->setPitch(sid, RandomFloat(-1.f, -0.25f));
}

void Apple::updateAnimated(float dt) {

	vector<int> frameOrder = { 0, 2, 0 }; // Order of using keyframes
	vector<float> frameTime = { 0.1f, 0.4f };

	// vector<int> frameOrder = { 1, 2, 0, 1 }; // Order of using keyframes
	// vector<float> frameTime = { 0.1f, 0.2f, 0.2f };

	float animLoopTime = 0;
	for (size_t i = 0; i < frameTime.size(); i++)
		animLoopTime += frameTime[i];

	m_frameTime = Clamp<float>(m_frameTime + dt, 0, animLoopTime); // increment animation

	float time = frameTime[0];
	int frame = 0;
	while (frame + 1 < frameTime.size() && m_frameTime > time) {
		frame++;
		time += frameTime[frame];
	}
	float rest = (m_frameTime - (time - frameTime[frame])) / frameTime[frame];
	float smoothRest = -cos(rest * XM_PI) * 0.5f + 0.5f;
	m_meshAnim.updateSpecific(smoothRest);
	m_meshAnim.setFrameTargets(frameOrder[frame], frameOrder[frame + 1]);
}

void Apple::restartAnimation() { m_frameTime = 0; }

void Apple::update() {
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

bool Apple::isRespawning() const { return m_respawn_timer != 0; }

void Apple::_onDeath(Skillshot skillshot) { spawnCollectionPoint(skillshot); }

void Apple::behavior() {
	float dt = SceneManager::getScene()->getDeltaTime();

	// collision
	float3 ray_point = getBoundingBoxPos() - float3(0, 1, 0) * getHalfSizes();
	float3 ray_distance = m_velocity * dt;
	float3 intersection_position, intersection_normal;
	if (rayCastWorld(ray_point, ray_distance, intersection_position, intersection_normal)) {
		intersection_normal.Normalize();
		float3 up(0, 1, 0);
		if (intersection_normal.Dot(up) >= 0.7f) {
			// flat
			// jump
			float3 target = findJumpLocation(5, 3, 16, 10);
			jumpToLocation(target, float2(1.f, 1.5f));
			playSound_bounce();
			// set looking direction
			float3 flatVel = m_velocity * float3(1, 0, 1);
			if (flatVel.Length() > 0) {
				m_desiredLookDirection = Normalize(flatVel);
				// lookTo(flatVel);
			}
			// reset animation
			m_frameTime = 0;
		}
		else {
			// steep
			// slide downhill
			m_velocity -= m_velocity.Dot(intersection_normal) *
						  intersection_normal; // remove force against normal
		}
	}

	// look towards desired direction
	m_currentLookDirection += (m_desiredLookDirection - m_currentLookDirection) *
							  Clamp<float>(1 - pow(rotationSpeed, dt), 0.f, 1.f);
	m_currentLookDirection.Normalize();
	lookTo(m_currentLookDirection);
}
