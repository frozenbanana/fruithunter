#pragma once
#include "State.h"
#include "Timer.h"
#include "TextRenderer.h"
#include "AudioHandler.h"
#include "ShadowMapping.h"
#include "AudioHandler.h"
#include "Entity.h"
#include "Animal.h"
#include "SceneManager.h"

class LevelSelectState : public State {
public:
	void initialize();
	void update();
	void handleEvent();
	void pause();
	void play();
	void draw();

	LevelSelectState() { initialize(); }
	~LevelSelectState();

private:
	TextRenderer m_textRenderer;
	Timer m_timer;

	struct LevelSelector {
		Entity m_bowl;
		Entity m_content;
	};
	vector<LevelSelector> m_levelSelectors;
	std::vector<shared_ptr<Animal>> m_animal;
	std::vector<string> m_maps;

	SceneManager sceneManager;

	void setLevel(int);
	void initializeLevelSelectors();
};