#include "LevelSelectState.h"
#include "ErrorLogger.h"
#include "Renderer.h"

void LevelSelectState::initialize() {
	m_name = "Level select state";

	//Initiate player
	m_player.initialize();

	//Initiate terrain
	m_terrainManager = TerrainManager::getInstance();

	//Initiate props


	//Initiate water
	m_waterEffect.initilize(SeaEffect::SeaEffectTypes::water, XMINT2(400, 400), XMINT2(1, 1),
		float3(0.f, 1.f, 0.f) - float3(100.f, 0.f, 100.f), float3(400.f, 2.f, 400.f));

	//Initiate sky box

	//Initiate fruit bowls

	m_shadowMap = make_unique<ShadowMapper>();
}

void LevelSelectState::update() {}

void LevelSelectState::handleEvent() { return; }

void LevelSelectState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::getInstance()->pauseAllMusic();
}

void LevelSelectState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");
	Renderer::getInstance()->drawLoading();
}
