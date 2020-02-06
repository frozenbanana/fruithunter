#include "PlayState.hpp"
#include "ErrorLogger.hpp"
#include "Renderer.hpp"
#include "Quad.hpp"
#include <iostream>
#include <string>

using Vector2 = DirectX::SimpleMath::Vector2;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector4 = DirectX::SimpleMath::Vector4;

void PlayState::initialize() {
	m_name = "Play State";

	m_terrain.initilize("heightmap1.png", XMINT2(50, 50), XMINT2(1,1));
	m_terrain.setScale(float3(1, 0.25, 1) * 25);

	m_player.initialize();
	m_player.setPosition(float3(1, 1, 1));

	m_bow.loadAnimated("Bow", 3);
	m_bow.setPosition(float3(2.f, 0.f, 0.f));
}

void PlayState::update() {
	float3 pos = m_player.getPosition();
	float3 normal = m_terrain.getNormalFromPosition(pos.x, pos.z);
	float h = m_terrain.getHeightFromPosition(pos.x, pos.z);
	m_player.update(0.017f, h + 0.5f, normal);
	m_timer.update();
	float dt = m_timer.getDt();
	m_player.update(dt);
	m_apple.updateAnimated(dt);
	m_bow.updateAnimated(dt);
}

void PlayState::handleEvent() { return; }

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::pauseAmbient();
}

void PlayState::draw() {
	m_player.draw();

	m_terrain.draw();

		// Text
	float t = m_timer.getTimePassed();
	Vector4 col = Vector4(.5f, abs(cos(t)), abs(sin(t)), 1.f);
	m_textRenderer.draw("HERE IS THE GOAT", Vector2(400., 300.), col);

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
