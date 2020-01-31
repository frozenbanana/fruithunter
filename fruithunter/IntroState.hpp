#pragma once
#include "State.hpp"
#include "TextRenderer.hpp"
#include "Timer.hpp"

class IntroState : public State {
public:
	IntroState();
	void update();
	void initialize();
	void handleEvent(int event);
	void pause();
	void play();
	void draw();

private:
	Timer m_timer;
	TextRenderer m_textRenderer;
};
