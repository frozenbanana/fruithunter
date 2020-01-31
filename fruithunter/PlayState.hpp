#pragma once
#include "State.hpp"
#include "Quad.hpp"
#include "Camera.hpp"
#include "Timer.hpp"
#include "TextRenderer.hpp"


class PlayState : public State {
public:
	void update();
	void initialize();
	void handleEvent(int event);
	void pause();
	void play();
	void draw();

	PlayState() { initialize(); }

private:
	Timer m_timer;
	TextRenderer m_textRenderer;
	Camera m_camera;
	Quad m_quad;
};