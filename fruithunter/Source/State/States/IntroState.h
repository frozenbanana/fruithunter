#pragma once
#include "State.h"
#include "Entity.h"
#include "Apple.h"
#include "Camera.h"
#include "TextRenderer.h"
#include "Timer.h"

class IntroState : public State {
public:
	IntroState();
	~IntroState();
	void update();
	void initialize();
	void handleEvent();
	void pause();
	void play();
	void draw();
	void drawShadow();

private:
	Camera m_camera;
	Timer m_timer;
	Entity m_entity;
	std::vector<Apple> m_apples;

	TextRenderer m_textRenderer;
};
