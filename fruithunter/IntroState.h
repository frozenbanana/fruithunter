#pragma once
#include "State.h"
#include "Entity.h"
#include "Apple.h"
#include "Camera.h"
#include <string>
#include "Input.h"
#include "TextRenderer.h"
#include "Timer.h"
#include "AudioHandler.h"
#include "Terrain.h"
#include "Apple.h"

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

	TextRenderer m_textRenderer;
};
