#pragma once
#include "StateItem.h"
#include "SceneManager.h"

class PlayState : public StateItem {
private:
	SceneManager sceneManager;
	Keyboard::Keys m_mouseMode_switch = Keyboard::H;
	bool m_mouseMode = true;

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