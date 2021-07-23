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

	void init() override;
	void update(double dt) override;
	void draw() override;

	void play() override;
	void pause() override;
	void restart() override;

	void changeScene(string sceneName);

};