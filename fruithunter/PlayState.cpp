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
	// if (!m_isLoaded) {
	m_name = "Play State";

	m_terrain.initilize("heightmap1.png", XMINT2(50,50), XMINT2(10,10));
	m_terrain.setScale(float3(1, 0.25, 1) * 25);

	m_entity.load("sphere");
	m_entity.setScale(0.1);

	m_player.initialize();
	m_player.setPosition(float3(1, 1, 1));

	// Timer
	// TODO: Refactor to a static timeHandler
	LARGE_INTEGER timer;
	if (!QueryPerformanceCounter(&timer)) {
		ErrorLogger::log("Cannot query performance counter in " + m_name + ".");
		return;
	}

	// m_frequencySeconds = (float)(timer.QuadPart);
	// Get Current value
	QueryPerformanceCounter(&timer);
	// m_startTime = timer.QuadPart;
	// m_totalTime = 0.;
	// m_elapsedTime = 0.;

	// m_isLoaded = true;
	//}
}

void PlayState::update() {
	float3 pos = m_player.getPosition();
	float3 normal = m_terrain.getNormalFromPosition(pos.x, pos.z);
	float h = m_terrain.getHeightFromPosition(pos.x, pos.z);
	m_player.update(0.017f, h + 0.5, normal);

}

void PlayState::handleEvent() { return; }

void PlayState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::pauseAmbient();
}

void PlayState::draw() {
	m_player.draw();

	m_terrain.draw();

	float3 p = m_player.getPosition();
	float3 d = m_player.getForward()*10;
	m_entity.setPosition(p + d);
	m_entity.draw();
	m_terrain.castRay(p, d);
	//ErrorLogger::log(to_string(p.x) + ":" + to_string(p.y) + ":" + to_string(p.z));
	m_entity.setPosition(p);
	m_entity.draw();

	m_entity.setPosition(p+d);
	//m_entity.draw();

		// Text
	float t = m_timer.getTimePassed();
	Vector4 col = Vector4(.5f, abs(cos(t)), abs(sin(t)), 1.f);
	// m_textRenderer.draw("HERE IS THE GOAT", Vector2(400., 300.), col);
}

void PlayState::play() {
	AudioHandler::startPlayAmbient();
	ErrorLogger::log(m_name + " play() called.");
}
