#pragma once
#include "StateItem.h"
#include "SceneManager.h"

class PlayState : public StateItem {
private:
	SceneManager sceneManager;

public:
	PlayState();

	void init();
	void update();
	void draw();

	void play();
	void pause();
	void restart();

	void changeScene(string sceneName);

};