#pragma once
#include "State.h"
#include "Quad.h"
#include "Player.h"
#include "Timer.h"
#include "TextRenderer.h"
#include "AudioHandler.h"
#include "Entity.h"
#include "Terrain.h"
#include "Apple.h"
#include "SkyBox.h"
#include "Banana.h"

class PlayState : public State {
public:
	void update();
	void initialize();
	void handleEvent();
	void pause();
	void play();
	void draw();

	PlayState() { initialize(); }

private:
	Timer m_timer;
	TextRenderer m_textRenderer;
	Player m_player;
	Quad m_quad;
	Apple m_apple;
	Terrain m_terrain;
	Entity m_entity;
	Entity m_bow;
	SkyBox m_skyBox;
	Banana m_banana;
};