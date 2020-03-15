#pragma once
#include "State.h"
//#include "Quad.h"
#include "Timer.h"
#include "TextRenderer.h"
#include "AudioHandler.h"
#include "LevelHandler.h"
#include "ShadowMapping.h"
#include "AudioHandler.h"
#include "Entity.h"
#include "Animal.h"

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
	float3 m_spawnPosition = float3(34.0f, 2.5f, 79.9f);
	TextRenderer m_textRenderer;
	Timer m_timer;
	Player m_player;
	EntityRepository m_terrainProps;
	SeaEffect m_waterEffect;
	SkyBox m_skyBox;
	vector<unique_ptr<Entity>> m_levelBowls;
	std::vector<shared_ptr<Animal>> m_animal;
	std::vector<string> m_maps;

	void setLevel(int);
};