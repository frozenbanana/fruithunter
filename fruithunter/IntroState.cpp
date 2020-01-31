#include "IntroState.hpp"
#include "ErrorLogger.hpp"

IntroState::IntroState() { initialize(); }

IntroState::~IntroState() { 
}

void IntroState::initialize() {
	m_name = "Intro State";

	m_entity.load("treeMedium1");

	m_camera.setView(Vector3(0.0, 0.0, -10.0), Vector3(0.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0));
	m_camera.createBuffer();
	m_camera.buildMatrices();
	m_camera.updateBuffer();
}

void IntroState::update() {
	// ErrorLogger::log(m_name + " update() called.");

	// float t = clock()/1000;
	float rotSpeed = 1;
	rot += 0.01;
	m_entity.setPosition(float3(5*sin(rot),0,0));
	m_entity.rotateY(3.14f*1.f/60.f);
	m_entity.setScale(sin(rot));
	// rot += rotSpeed*(Input::getInstance()->keyDown(Keyboard::A) -
	// Input::getInstance()->keyDown(Keyboard::D));
	//m_camera.setEye(Vector3(-10 * sin(rot), 1, -10 * cos(rot)));
	m_camera.buildMatrices();
	m_camera.updateBuffer();
}

void IntroState::handleEvent(int event) { return; }

void IntroState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void IntroState::draw() {
	// ErrorLogger::log(m_name + " draw() called.");

	m_camera.bindMatix();

	// m_entity.draw_onlyMesh(float3(1, 0, 0));
	if(Input::getInstance()->keyDown(Keyboard::Space))m_entity.draw_boundingBox();
	m_entity.draw();
}

void IntroState::play() { ErrorLogger::log(m_name + " play() called."); }
