#pragma once
#include "State.hpp"
#include "Quad.hpp"
//#include "Camera.hpp"
#include "Player.hpp"
#include "Timer.hpp"
#include "TextRenderer.hpp"
#include "AudioHandler.hpp"


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
	TextRenderer m_textRenderer;
	Player m_player;
	Quad m_quad;
};