#pragma once
#include "State.h"
#include "Quad.h"
#include "Timer.h"
#include "TextRenderer.h"
#include "AudioHandler.h"
#include "LevelHandler.h"
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

private:
	Timer m_timer;
	TextRenderer m_textRenderer;
	//Player m_player;
	Quad m_quad;
	LevelHandler m_levelHandler;
	//Entity m_bow;
};