#pragma once
#include "State.hpp"
#include "Quad.hpp"
#include "Camera.hpp"
#include "Timer.hpp"

using namespace DirectX::SimpleMath;

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
	Camera m_camera;
	Quad m_quad;
};