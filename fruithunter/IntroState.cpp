#include "IntroState.hpp"
#include "ErrorLogger.hpp"
#include <SimpleMath.h>
#include <string>

using Vector2 = DirectX::SimpleMath::Vector2;

IntroState::IntroState() { initialize(); }

IntroState::~IntroState() {}

void IntroState::initialize() {
	m_name = "Intro State";

	m_entity.load("bow");

	terrain.initilize("heightmap2.png",XMINT2(100,100),XMINT2(2,2));

	m_camera.setView(Vector3(0.0, 5.0, -10.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));
	m_camera.createBuffer();
	m_camera.buildMatrices();
	m_camera.updateBuffer();
}

void IntroState::update() {
	// ErrorLogger::log(m_name + " update() called.");
	m_timer.update();
	float rotSpeed = 1;
	rot += 0.01;
	m_entity.setPosition(float3(5 * sin(rot), 0, 0));
	m_entity.rotateY(3.14f * 1.f / 60.f);
	m_entity.setScale(sin(rot));

	m_camera.buildMatrices();
	m_camera.updateBuffer();

	terrain.rotateY(0.03*0.1);
}

void IntroState::handleEvent(int event) { return; }

void IntroState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void IntroState::draw() {
	// ErrorLogger::log(m_name + " draw() called.");
	float t = m_timer.getTimePassed();
	Vector4 col = Vector4(abs(sin(t)), .5f, abs(cos(t)), 1.f);
	//m_textRenderer.draw("LET ME SEE THE GOAT " + std::to_string(t), Vector2(400., 300.), col);

	m_camera.bindMatix();

	if (Input::getInstance()->keyDown(Keyboard::Space))
		m_entity.draw_boundingBox();
	//m_entity.draw();

	terrain.draw();


}

void IntroState::play() { ErrorLogger::log(m_name + " play() called."); }
