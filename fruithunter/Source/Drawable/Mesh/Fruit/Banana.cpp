#include "Banana.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "SceneManager.h"

Banana::Banana(float3 pos) : Fruit(FruitType::BANANA, pos) {
	loadAnimated("Banana", 3);
	setCollisionDataOBB();

	setScale(2.f);

	setRotation(float3(RandomFloat(), RandomFloat(), RandomFloat()) * 2 * XM_PI);

	jumpToLocation(getPosition());

	// no friction (difficult jump calculation if there are)
	m_groundFriction = 60;
	m_airFriction = 60;

	m_gravity = float3(0.0f, -1.0f, 0.0f) * 25.0f;
}

void Banana::restartAnimation() { m_frameTime = 0; }

void Banana::updateAnimated(float dt) {
	vector<int> frameOrder = { 1, 2, 0, 1 }; // Order of using keyframes
	vector<float> frameTime = { 0.1f, 0.2f, 0.2f };

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
	// rest = -cos(rest * XM_PI) * 0.5f + 0.5f; // smooth rest
	m_meshAnim.updateSpecific(rest);
	m_meshAnim.setFrameTargets(frameOrder[frame], frameOrder[frame + 1]);

	setRotation(getRotation() + float3(1, 1, 1) * 2 * dt);
}

void Banana::update() {
	Scene* scene = SceneManager::getScene();
	float dt = scene->getDeltaTime();

	checkOnGroundStatus(); // checks if on ground
	behavior();
	updateAnimated(dt);	   // animation stuff
	updateVelocity(dt);	   // update velocity (slowdown and apply accelration)
	move(dt);			  // update position from velocity
	enforceOverTerrain(); // force fruit above ground

	updateRespawn();
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
			float3 target = findJumpLocation(6, 6, 16, 10);
			jumpToLocation(target, float2(2, 5));
			restartAnimation();
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
