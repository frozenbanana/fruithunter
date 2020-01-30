#include "IntroState.hpp"
#include "ErrorLogger.hpp"

IntroState::IntroState() { initialize(); }

void IntroState::initialize() {
	m_name = "Intro State";
	entity.load("treeMedium1");
}

void IntroState::update() { ErrorLogger::log(m_name + " update() called."); }

void IntroState::handleEvent(int event) { return; }

void IntroState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void IntroState::draw() {
	ErrorLogger::log(m_name + " draw() called.");

	auto deviceContext = Renderer::getDeviceContext();

	deviceContext->OMSetDepthStencilState(Renderer::getInstance()->m_depthState.Get(), 1);

	entity.draw_onlyMesh(float3(1, 0, 0));
}

void IntroState::play() { ErrorLogger::log(m_name + " play() called."); }
