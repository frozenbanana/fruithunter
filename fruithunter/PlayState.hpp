#pragma once
#include "State.hpp"
#include "Quad.hpp"
#include "Player.hpp"
#include "Timer.hpp"
#include "TextRenderer.hpp"
#include "AudioHandler.hpp"
#include "Entity.hpp"
#include "Terrain.hpp"
#include "Apple.hpp"


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
};