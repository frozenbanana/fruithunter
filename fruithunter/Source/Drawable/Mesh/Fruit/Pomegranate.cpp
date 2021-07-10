#include "Pomegranate.h"
#include "Input.h"
#include "SceneManager.h"

void Pomegranate::behavior(float dt) {
	Scene* scene = SceneManager::getScene();
	float3 playerPosition = scene->m_player->getPosition();

	if (m_jumping) {
		// in air
		m_jumpAnim = Clamp(m_jumpAnim + dt, 0.f, 1.f);
		m_frameTime = (1.f / 2.2f) + m_jumpAnim;

		if (m_jumpAnim == 1.f) {
			m_jumping = false;
			m_jumpAnim = 0;
		}
	}
	else {
		// on ground
		m_charge = Clamp(m_charge + dt, 0.f, 1.f);
		m_frameTime = m_charge / 2.2f;
		if (m_charge >= 1) {
			// jump
			m_charge = 0;
			m_jumping = true;

			float3 dir = Normalize(float3(0, 1, 0));
			m_velocity = dir * 10;
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

	float3 movement = m_velocity * dt;
	float3 nextPosition = getPosition() + movement;
	float height = SceneManager::getScene()->m_terrains.getHeightFromPosition(nextPosition);
	if (nextPosition.y < height) {
		m_velocity.y = 0;
	}

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

Pomegranate::Pomegranate(float3 position) : Fruit(FruitType::POMEGRANATE, position) {
	loadAnimated("pomegranate", 3);
	setFrameTargets(0, 1);

	setCollisionDataOBB();

	setScale(0.75f);

	m_speed = 0;
	m_groundFriction = 60.f;
	m_airFriction = 60.f;
}
