#pragma once
#include "State.hpp"
#include "Quad.hpp"
#include "Camera.hpp"

using namespace DirectX::SimpleMath;

class PlayState : public State {
public:
	void update();
	void initialize();
	void handleEvent(int event);
	void pause();
	void play();
	void draw();

	static PlayState* getInstance() { return &m_playState; }

protected:
	PlayState() {}

private:
	static PlayState m_playState;
	LARGE_INTEGER m_timer;
	float m_frequencySeconds;
	float m_startTime;
	float m_totalTime;
	float m_elapsedTime;
	Camera m_camera;
	Quad m_quad;
	bool m_isLoaded = false;
};