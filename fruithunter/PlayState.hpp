#pragma once
#include "State.hpp"
#include "Quad.hpp"
//#include "Camera.hpp"
#include "Player.hpp"
#include "Timer.hpp"
#include "TextRenderer.hpp"
#include "AudioHandler.hpp"
#include "Entity.h"
#include "Terrain.h"


class PlayState : public State
{
public:
	void update();
	void initialize();
	void handleEvent();
	void pause();
	void play();
	void draw();

	PlayState()
	{
		initialize();
	}

private:
	Timer m_timer;
	//TextRenderer m_textRenderer;
	Player m_player;
	Terrain m_terrain;
	Entity m_entity;
};