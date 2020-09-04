#include "PlayState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Input.h"
#include "EndRoundState.h"
#include "SaveManager.h"

void PlayState::init() {}

void PlayState::update() {
	Input::getInstance()->setMouseModeRelative();

	sceneManager.update();

	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		push(State::PauseState);
	}

	if (SceneManager::getScene()->handleWin()) {
		push(State::EndRoundState);
	}
}

void PlayState::draw() {
	sceneManager.setup_shadow();
	sceneManager.draw_shadow();
	sceneManager.setup_color();
	sceneManager.draw_color();
	sceneManager.draw_hud();
}

void PlayState::play() {}

void PlayState::pause() {}

void PlayState::changeScene(string sceneName) { sceneManager.load(sceneName); }

void PlayState::restart() { sceneManager.reset(); }

PlayState::PlayState() : StateItem(StateItem::State::PlayState) {}