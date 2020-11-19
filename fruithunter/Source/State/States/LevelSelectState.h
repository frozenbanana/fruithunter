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

	Entity m_objTest;
	float3 plane_point = float3(32.557, 6.254, 75.833);
	float plane_heightOffset = 0;
	float3 plane_normal = float3(0, -1, 0);
	float3 plane_color = float3(1, 1, 0);

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