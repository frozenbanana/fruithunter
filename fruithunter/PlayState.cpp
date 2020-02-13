#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Quad.h"
#include <iostream>
#include <string>

using Vector2 = DirectX::SimpleMath::Vector2;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector4 = DirectX::SimpleMath::Vector4;

void PlayState::initialize() {
	m_name = "Play State";

	m_terrainManager.add(float3(0, 0, 0), "heightmap3.jpg", XMINT2(50, 50), XMINT2(5, 5));
	m_terrainManager.add(float3(10, 0, 0), "heightmap3.jpg", XMINT2(50, 50), XMINT2(5, 5));
	m_terrainManager.add(float3(0, 0, 10), "heightmap3.jpg", XMINT2(50, 50), XMINT2(5, 5));
	m_terrainManager.add(float3(10, 0, 10), "heightmap3.jpg", XMINT2(50, 50), XMINT2(5, 5));

	m_entity.load("sphere");
	m_entity.setScale(0.1f);

	m_player.initialize();
	m_player.setPosition(float3(2.f, 0.f, 0.2f));

	m_bow.loadAnimated("Bow", 3);
	m_bow.setPosition(float3(2.f, 0.f, 0.f));
}

void PlayState::update() {
	float3 pos = m_player.getPosition();
	Terrain* terrain = m_terrainManager.getTerrainFromPosition(pos);
	float3 normal;
	float height = 0;
	if (terrain != nullptr) {
		normal = terrain->getNormalFromPosition(pos.x, pos.z);
		height = terrain->getHeightFromPosition(pos.x, pos.z);
	}

	m_player.update(0.017f, height + 0.5f, normal);
	m_timer.update();
	float dt = m_timer.getDt();
	m_bow.updateAnimated(dt);

	m_apple.updateAnimated(dt);
	float3 appleDestination =
		float3(sin(m_timer.getTimePassed() * 0.6f), 0.f, cos(m_timer.getTimePassed() * 0.6f)) *
		5.0f;
	m_apple.setNextDestination(appleDestination);
}

void PlayState::handleEvent() { return; }

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::pauseAmbient();
}

void PlayState::draw() {
	m_player.draw();

	m_terrainManager.draw();

	float l = m_terrainManager.castRay(m_player.getPosition(),m_player.getForward()*10);
	float3 p = m_player.getPosition() + m_player.getForward() * 10 * l;
	m_entity.setPosition(p);
	m_entity.draw();

	// Text
	float t = m_timer.getTimePassed();
	Vector4 col = Vector4(.5f, abs(cos(t)), abs(sin(t)), 1.f);
	m_textRenderer.draw("Time: " + std::to_string(t), Vector2(400., 75.), col);

	// Apple
	m_apple.draw_animate();

	// Bow
	m_bow.draw_animate();
}

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	AudioHandler::startPlayAmbient();
	ErrorLogger::log(m_name + " play() called.");
}
