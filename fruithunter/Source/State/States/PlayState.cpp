#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Quad.h"
#include <iostream>
#include <string>

void PlayState::initialize() {
	m_name = "Play State";

	m_terrainManager.add(float3(0, 0, 0), "heightmap3.jpg", XMINT2(50, 50), XMINT2(5, 5));
	m_terrainManager.add(float3(10, 0, 0), "heightmap3.jpg", XMINT2(50, 50), XMINT2(5, 5));
	m_terrainManager.add(float3(0, 0, 10), "heightmap3.jpg", XMINT2(50, 50), XMINT2(5, 5));
	m_terrainManager.add(float3(10, 0, 10), "heightmap3.jpg", XMINT2(50, 50), XMINT2(5, 5));

	// m_entity.load("sphere");
	// m_entity.setScale(0.1f);

	m_player.initialize();
	m_player.setPosition(float3(2.f, 0.f, 0.2f));

	m_bow.loadAnimated("Bow", 3);
	m_bow.setPosition(float3(2.f, 0.f, 0.f));

	m_player.setPosition(float3(13.f));
	float3 bananaPos(12.f);
	bananaPos = float3(bananaPos.x, m_terrainManager.getHeightFromPosition(bananaPos), bananaPos.z);
	m_banana.setStartPosition(bananaPos);
	m_melon.setPosition(float3(-1.f));
}

void PlayState::update() {


	m_timer.update();
	float dt = m_timer.getDt();

	m_player.update(dt, m_terrainManager.getTerrainFromPosition(m_player.getPosition()));
	// m_bow.updateAnimated(dt);

	// update apple
	m_apple.update(dt, m_player.getPosition());
	// m_apple.update(dt,
	//	m_terrain.getHeightFromPosition(m_apple.getPosition().x, m_apple.getPosition().z),
	//	m_terrain.getNormalFromPosition(m_apple.getPosition().x, m_apple.getPosition().z), pos);


	// update banana
	float3 bounceDir = m_terrainManager.getNormalFromPosition(m_banana.getPosition());
	bounceDir.y = 0;
	bounceDir.Normalize();
	bounceDir *= 3;
	bounceDir += m_banana.getPosition();
	bounceDir.y = m_terrainManager.getHeightFromPosition(bounceDir);
	m_banana.setNextDestination(bounceDir);
	m_banana.updateAnimated(dt);

	// update melon
	m_melon.setNextDestination(m_player.getPosition());
	float3 melonPos = m_melon.getPosition();
	melonPos.y = m_terrainManager.getHeightFromPosition(melonPos);
	m_melon.setPosition(melonPos);
	m_melon.updateAnimated(dt);

	// collisionTest


	if (m_player.getArrow().checkCollision(m_melon)) {
		m_melon.setPosition(float3(0.f));
		m_player.getArrow().setPosition(float3(-1.f));
	}
}

void PlayState::handleEvent() { return; }

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::pauseAmbient();
}

void PlayState::draw() {
	m_player.draw();
	m_terrainManager.draw();

	/*
	float3 p = m_player.getPosition();
	float3 d = m_player.getForward() * 10;
	m_entity.setPosition(p + d);
	m_entity.draw();
	m_terrain.castRay(p, d);
	m_entity.setPosition(p);
	m_entity.draw();
	*/

	// Apple
	m_apple.draw_animate();
	// Banana
	m_banana.draw_animate();
	// Melon
	m_melon.draw_animate();
	// Bow
	m_bow.draw_animate();

	// SkyBox
	m_skyBox.draw();

	// Text
	float t = m_timer.getTimePassed();
	Vector4 col = Vector4(.5f, abs(cos(t)), abs(sin(t)), 1.f);
	m_textRenderer.draw("Time: " + std::to_string(t), Vector2(400., 75.), col);
}

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	AudioHandler::startPlayAmbient();
	ErrorLogger::log(m_name + " play() called.");
}
