#pragma once
#include "State.hpp"
#include "Entity.h"
#include "Apple.hpp"
#include "Camera.hpp"
#include <string>
#include "Input.hpp"
#include "TextRenderer.hpp"
#include "Timer.hpp"
#include "AudioHandler.hpp"
#include "Terrain.h"

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
	TextRenderer m_textRenderer;
	Apple m_apple;
};
