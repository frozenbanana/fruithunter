#pragma once
#include "State.hpp"
#include "Quad.hpp"

class PlayState : public State {
public:
	void update();
	void init();
	void handleEvent(int event);
	void pause();
	void play();
	void draw();

	static PlayState* getInstance() { return &m_playState; }

protected:
	PlayState() {}

private:
	static PlayState m_playState;
	Quad m_quad;
};
