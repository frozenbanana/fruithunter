#pragma once
#include "State.h"
#include "Quad.h"
#include "Timer.h"
#include "TextRenderer.h"
#include "AudioHandler.h"
#include "SceneManager.h"

class PlayState : public State {
public:
	void update();
	void initialize();
	void handleEvent();
	void pause();
	void play();
	void draw();

	void changeScene(string sceneName);
	void restart();

	PlayState() { initialize(); }

private:

	SceneManager sceneManager;
};