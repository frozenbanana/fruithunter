#pragma once
#include "State.hpp"
#include "Entity.h"
#include "Camera.hpp"
#include <string>
#include <time.h>
#include "Input.hpp"

class IntroState : public State {
public:
	IntroState();
	~IntroState();
	void update();
	void initialize();
	void handleEvent(int event);
	void pause();
	void play();
	void draw();

private:
	float rot = 0;
	//clock_t m_timer;
	Camera m_camera;
	Entity m_entity;
};
