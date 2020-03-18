#include "PauseState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"
#include "AudioHandler.h"
#include "TerrainManager.h"
#include "PlayState.h"

PauseState::PauseState() { initialize(); }

PauseState::~PauseState() {}

void PauseState::initialize() {
	m_name = "Pause State";

	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;

	m_restartButton.initialize("Restart", float2(width / 2, height / 2 - 120));
	m_resumeButton.initialize("Resume", float2(width / 2, height / 2 - 60));
	m_settingsButton.initialize("Settings", float2(width / 2, height / 2));
	m_mainMenuButton.initialize("Main Menu", float2(width / 2, height / 2 + 60));
	m_exitButton.initialize("Exit", float2(width / 2, height / 2 + 120));

	m_settingsBackground.load("apple.png");
	m_settingsBackground.setPosition(float2(1280.f / 2.f, 720.f / 2.f));
	m_settingsBackground.setScale(1.25f);
	m_settingsBackground.setAlignment();//center

	float2 scale = float2(0.25f);
	float picWidth = 400 * scale.x;
	float pixHeight = 400 * scale.y;
	float padding = 50 * scale.x;
	float2 position = float2(10, 10);
	float animationSpeed = 0.5f;
	//pick fruits
	vector<vector<string>> strTextures = { 
		{ "drop_hold.png", "drop_releaseApple.png" },
		{ "drop_hold.png", "drop_releaseBanana.png" },
		{ "drop_hold.png", "drop_releaseMelon.png" },
		{ "drop_hold.png", "drop_releaseDragonFruit.png" }
	};
	for (size_t i = 0; i < 4; i++) {
		m_dropFruits[i].load(strTextures[i], animationSpeed);
		m_dropFruits[i].set(position, scale);
		position.x += picWidth + padding;
	}

	//sloMo
	vector<string> strSlowMo = { "hourGlass0.png", "hourGlass1.png" };
	m_slowMo.load(strSlowMo, animationSpeed);
	m_slowMo.set(position, scale);
	
	//Buttons
	position = float2(10, 10);
	position.y += pixHeight + padding;
	vector<vector<string>> strTextureButtons = { { "btn1.png", "btn1_pressed.png" },
		{ "btn2.png", "btn2_pressed.png" }, { "btn3.png", "btn3_pressed.png" },
		{ "btn4.png", "btn4_pressed.png" }, { "btnF.png", "btnF_pressed.png" } };
	for (size_t i = 0; i < strTextureButtons.size(); i++) {
		m_btns[i].load(strTextureButtons[i], animationSpeed);
		m_btns[i].set(position, scale);
		position.x += picWidth + padding;
	}

	// movement keys
	vector<string> strMovementTex = {
		"CharWalk0.png",
		"CharWalk1.png",
		"CharWalk2.png",
		"CharWalk1.png",
	};
	m_charMovement.load(strMovementTex, animationSpeed);
	m_charMovement.set(float2(1070, 140), float2(0.5f));
	m_charMovement.setAlignment();//center
	strMovementTex = { "movement.png", "movement_w.png", "movement_a.png", "movement_s.png", "movement_d.png" };
	m_movementKeys.load(strMovementTex, animationSpeed);
	m_movementKeys.set(float2(1070, 310), float2(0.5f));
	m_movementKeys.setAlignment();//center

	//jump key
	vector<string> strJumpTex = { "charJump0.png", "charJump1.png" };
	m_charJump.load(strJumpTex, animationSpeed);
	m_charJump.set(float2(1070, 500), float2(0.5));
	m_charJump.setAlignment();//Center
	strJumpTex = { "jump_up.png", "jump_down.png" };
	m_jumpKey.load(strJumpTex, animationSpeed);
	m_jumpKey.set(float2(1070, 620), float2(0.5));
	m_jumpKey.setAlignment();//Center

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));
}

void PauseState::update() {
	Input::getInstance()->setMouseModeAbsolute();
}

void PauseState::handleEvent() {
	if (m_resumeButton.update() || Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->resumeState();
	}
	if (StateHandler::getInstance()->getPreviousState() == StateHandler::PLAY &&
		m_restartButton.update()) {
		State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
		dynamic_cast<PlayState*>(tempPointer)->destroyLevel();
		TerrainManager::getInstance()->removeAll();
		StateHandler::getInstance()->changeState(StateHandler::PLAY);
	}
	if (m_settingsButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::SETTINGS);
	}
	if (m_mainMenuButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		StateHandler::getInstance()->changeState(StateHandler::INTRO);
	}
	if (m_exitButton.update()) {
		AudioHandler::getInstance()->pauseAllMusic();
		StateHandler::getInstance()->quit();
	}
}

void PauseState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void PauseState::play() {
	ErrorLogger::log(m_name + " play() called.");
	int previousState = StateHandler::getInstance()->getPreviousState();
	if (previousState == StateHandler::LEVEL_SELECT || previousState == StateHandler::PLAY)
		Renderer::getInstance()->captureFrame();

	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT + 60;

	m_restartButton.setPosition(float2(width / 2, height / 2 - 120));
	m_resumeButton.setPosition(float2(width / 2, height / 2 - 60));
	m_settingsButton.setPosition(float2(width / 2, height / 2));
	m_mainMenuButton.setPosition(float2(width / 2, height / 2 + 60));
	m_exitButton.setPosition(float2(width / 2, height / 2 + 120));

	//m_settingsBackground.setPosition(float2(width / 2.f, height / 2.f));
}


void PauseState::draw() {
	Renderer::getInstance()->beginFrame();
	Renderer::getInstance()->drawCapturedFrame();
	m_settingsBackground.drawNoScaling();

	if (StateHandler::getInstance()->getPreviousState() == StateHandler::PLAY)
		m_restartButton.draw();

	m_resumeButton.draw();
	m_mainMenuButton.draw();
	m_settingsButton.draw();
	m_exitButton.draw();

	for (size_t i = 0; i < 4; i++) {
		m_dropFruits[i].draw();
		m_btns[i].draw();
	}
	m_slowMo.draw();
	m_btns[4].draw();
	m_movementKeys.draw();
	m_charMovement.draw();
	m_jumpKey.draw();
	m_charJump.draw();

	// Just ignore this. It fixes things
	m_entity.draw();
}
