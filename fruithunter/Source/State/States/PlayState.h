#pragma once
#include "State.h"
#include "Quad.h"
#include "Timer.h"
#include "TextRenderer.h"
#include "AudioHandler.h"
#include "LevelHandler.h"
#include "ShadowMapping.h"
//#include "Entity.h"
//#include "TerrainManager.h"
//#include "Apple.h"
//#include "SkyBox.h"
//#include "Banana.h"
//#include "Melon.h"

class PlayState : public State {
public:
	void update();
	void initialize();
	void handleEvent();
	void pause();
	void play();
	void draw();
	void drawShadow();

	PlayState() { initialize(); }

	void setPlayerPos(float3);
	float3 getPlayerPos();

private:
	Timer m_timer;
	Quad m_quad;
	LevelHandler m_levelHandler;

	// Shadow stuff
	unique_ptr<ShadowMapper> m_shadowMap;
	float3 m_playerPos;
};