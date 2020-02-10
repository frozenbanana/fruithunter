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

	for (size_t i = 0; i < 4; i++) {
		m_bridges[i].load("bridge");
		m_bridges[i].setScale(0.15);
		m_bridges[i].setRotation(float3(0,(i+1)*(3.14/2),0));

	}
	
	m_bridges[0].setPosition(float3(10, 1, 5));
	m_bridges[1].setPosition(float3(5, 1, 10));
	m_bridges[2].setPosition(float3(10, 1, 15));
	m_bridges[3].setPosition(float3(15, 1, 10));

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

	for (int i = 0; i < 4; i++) {
		float l = m_bridges[i].castRay(pos, float3(0, -1, 0));
		if (l != -1) {
			ErrorLogger::log("HIT"+to_string(i));
			float h = (pos + float3(0, -1, 0) * l).y;
			if (h > height)
				height = h;
		}
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
	for (size_t i = 0; i < 4; i++) {
		m_bridges[i].draw_onlyMesh(float3(1,1,1));
	}

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
