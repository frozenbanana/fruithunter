#pragma once
#include "State.hpp"
#include "Entity.hpp"
#include "Apple.hpp"
#include "Camera.hpp"
#include <string>
#include "Input.hpp"
#include "TextRenderer.hpp"
#include "Timer.hpp"
#include "AudioHandler.hpp"
#include "Terrain.hpp"
#include "Apple.hpp"

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
