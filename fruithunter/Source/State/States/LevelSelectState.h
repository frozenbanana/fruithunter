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

#define NR_OF_LEVELS 3

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
	struct InWorldLevelBowl {
		Entity* m_bowl;
		size_t m_completionTime = 0;
		InWorldLevelBowl(float3 position) {
			m_bowl = new Entity("Bowl", position);
		}
	};
	InWorldLevelBowl m_levels[NR_OF_LEVELS] = { InWorldLevelBowl(float3(7.3f, 3.0f, 47.4f)),
		InWorldLevelBowl(float3(41.7f, 3.0f, 20.6f)),
		InWorldLevelBowl(float3(90.6f, 3.0f, 47.0f)) };
	std::vector<shared_ptr<Animal>> m_animal;
	std::vector<string> m_maps;

	void setLevel(int);
};