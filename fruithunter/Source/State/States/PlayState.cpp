#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Quad.h"
#include <iostream>
#include <string>

void PlayState::initialize() {
	m_name = "Play State";
}

void PlayState::update() {
	float3 playerPos = m_player.getPosition();
	float3 normal = m_terrainManager.getNormalFromPosition(playerPos);
	float h = m_terrainManager.getHeightFromPosition(playerPos);

	m_timer.update();
	float dt = m_timer.getDt();

	m_player.update(dt, m_terrainManager.getTerrainFromPosition(m_player.getPosition()));
	// m_bow.updateAnimated(dt);

	// update apple
	m_apple.update(dt, playerPos, &m_terrainManager);


	// update melon
	m_melon.update(dt, playerPos, &m_terrainManager);
	if (Input::getInstance()->keyDown(DirectX::Keyboard::M)) {
		m_player.setPosition(float3(2.f, 0.f, 2.0f));
	}
	// update melon
	// m_melon.setNextDestination(m_player.getPosition());

	// float3 melonPos = m_melon.getPosition();
	// melonPos.y = m_terrain.getHeightFromPosition(melonPos.x, melonPos.z);
	// m_melon.setPosition(melonPos);

	// update banana
	if (Input::getInstance()->keyDown(DirectX::Keyboard::Enter)) {
		m_banana.hit();
		m_melon.hit();
		m_apple.hit();
	}
	m_banana.update(dt, playerPos, &m_terrainManager);
}

void PlayState::handleEvent() { return; }

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::pauseAmbient();
}

void PlayState::draw() {
	m_levelHandler.draw();

	// Text
	float t = m_timer.getTimePassed();
	Vector4 col = Vector4(.5f, abs(cos(t)), abs(sin(t)), 1.f);
	m_textRenderer.draw("Time: " + std::to_string(t), Vector2(400., 75.), col);
}

void PlayState::play() {
	Input::getInstance()->setMouseModeRelative();
	AudioHandler::startPlayAmbient();
	ErrorLogger::log(m_name + " play() called.");
	m_levelHandler.loadLevel(0);
}
