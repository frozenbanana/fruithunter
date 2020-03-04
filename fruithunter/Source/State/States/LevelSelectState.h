#pragma once
#include "State.h"
//#include "Quad.h"
//#include "Timer.h"
//#include "TextRenderer.h"
#include "AudioHandler.h"
#include "LevelHandler.h"
#include "ShadowMapping.h"
//#include "Entity.h"
//#include "TerrainManager.h"
//#include "Apple.h"
//#include "SkyBox.h"
//#include "Banana.h"
//#include "Melon.h"

class LevelSelectState : public State {
public:
	void initialize();
	void update();
	void handleEvent();
	void pause();
	void play();
	void draw();

	LevelSelectState() { initialize(); }

private:
	Player m_player;
	EntityRepository m_terrainProps;
	SeaEffect m_waterEffect;
	SkyBox m_skyBox;
	TerrainManager* m_terrainManager;

	// Shadow stuff
	unique_ptr<ShadowMapper> m_shadowMap;
	bool m_staticShadowNotDrawn = true;
};