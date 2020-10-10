#pragma once
#include "StateItem.h"
#include "TextRenderer.h"
#include "SceneManager.h"

class LevelSelectState : public StateItem {
private:
	TextRenderer m_textRenderer;
	Timer m_timer;

	struct LevelSelector {
		string timeString;
		Entity m_bowl;
		Entity m_content;
	};
	vector<LevelSelector> m_levelSelectors;
	std::vector<shared_ptr<Animal>> m_animal;
	std::vector<string> m_maps;

	SceneManager sceneManager;

	Keyboard::Keys m_mouseMode_switch = Keyboard::H;
	bool m_mouseMode = true;

	void setLevel(int);
	void initializeLevelSelectors();

public:
	LevelSelectState();
	~LevelSelectState();

	void init();
	void update();
	void draw();

	void play();
	void pause();
	void restart();

};