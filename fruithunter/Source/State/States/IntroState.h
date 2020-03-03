#pragma once
#include "State.h"
#include "Entity.h"
#include "Apple.h"
#include "Camera.h"
#include "TextRenderer.h"
#include "Timer.h"
#include "ParticleSystem.h"


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

private:
	Camera m_camera;
	Timer m_timer;
	Entity m_entity;
	std::vector<Apple> m_apples;
	ParticleSystem m_particleSystem;

	TextRenderer m_textRenderer;
};
