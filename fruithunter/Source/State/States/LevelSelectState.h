#pragma once
#include "State.h"
//#include "Quad.h"
#include "Timer.h"
//#include "TextRenderer.h"
#include "AudioHandler.h"
#include "LevelHandler.h"
#include "ShadowMapping.h"
#include "AudioHandler.h"
//#include "Entity.h"
//#include "TerrainManager.h"
//#include "Apple.h"
//#include "SkyBox.h"
//#include "Banana.h"
//#include "Melon.h"
#include "Entity.h"

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
	Timer m_timer;
	Player m_player;
	EntityRepository m_terrainProps;
	SeaEffect m_waterEffect;
	SkyBox m_skyBox;
	Terrain* m_terrain;
	Entity* m_bowls[NR_OF_LEVELS];
	float3 m_bowlPos[NR_OF_LEVELS] = { 
		float3(11.5f, 6.2f, 40.8f), 
		float3(41.5f, 2.5f, 19.7f),
		float3(94.3f, 25.0f, 4.2f)
	};

	void setLevel(int);

	// Shadow stuff
	unique_ptr<ShadowMapper> m_shadowMap;
	bool m_staticShadowNotDrawn = true;
};