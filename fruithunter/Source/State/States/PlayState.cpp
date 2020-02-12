#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Quad.h"
#include <iostream>
#include <string>

void PlayState::initialize() {
	m_name = "Play State";

	m_terrain.initilize("heightmap1.png", XMINT2(50, 50), XMINT2(10, 10));
	m_terrain.setScale(float3(1, 0.25, 1) * 25);

	// m_entity.load("sphere");
	// m_entity.setScale(0.1f);

	m_player.initialize();
	m_player.setPosition(float3(2.f, 0.f, 0.2f));

	m_bow.loadAnimated("Bow", 3);
	m_bow.setPosition(float3(2.f, 0.f, 0.f));

	m_player.setPosition(float3(5.f, 5.f, 5.f));
	float3 bananaPos(12.f); // middle of terrain ish..
	bananaPos =
		float3(bananaPos.x, m_terrain.getHeightFromPosition(bananaPos.x, bananaPos.z), bananaPos.z);
	m_banana.setStartPosition(bananaPos);
	m_melon.setPosition(float3(-1.f));


	m_apple.setPosition(float3(1.f, 1.f, 1.f));
	m_apple.setPosition(float3(m_apple.getPosition().x,
		m_terrain.getHeightFromPosition(m_apple.getPosition()), m_apple.getPosition().z));
}

void PlayState::update() {
	float3 playerPos = m_player.getPosition();
	float3 normal = m_terrain.getNormalFromPosition(playerPos.x, playerPos.z);
	float h = m_terrain.getHeightFromPosition(playerPos.x, playerPos.z);

	m_timer.update();
	float dt = m_timer.getDt();

	m_player.update(dt, h + 1.5f, normal);
	// m_bow.updateAnimated(dt);

	// update apple
	float appleHeight = 0.1f + m_terrain.getHeightFromPosition(m_apple.getPosition());
	/*
	float x = m_apple.getPosition().x;
	float z = m_apple.getPosition().z;
	float3 newApplePosition = float3(x, appleHeight, z);

	if (m_apple.getState() != AI::State::ACTIVE) {
		m_apple.setNextDestination(newApplePosition);
	}*/
	m_apple.update(dt, playerPos, appleHeight);


	// update banana
	float3 bounceDestination =
		m_terrain.getNormalFromPosition(m_banana.getPosition().x, m_banana.getPosition().y);
	bounceDestination.y = 0;
	bounceDestination.Normalize();
	bounceDestination *= 3;
	bounceDestination += m_banana.getPosition();
	bounceDestination.y = m_terrain.getHeightFromPosition(bounceDestination.x, bounceDestination.z);
	m_banana.setNextDestination(bounceDestination);
	m_banana.updateAnimated(dt);
}

void PlayState::handleEvent() { return; }

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::pauseAmbient();
}

void PlayState::draw() {
	m_player.draw();
	m_terrain.draw();

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
