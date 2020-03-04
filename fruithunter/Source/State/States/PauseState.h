#pragma once
#include "State.h"
#include "Camera.h"
#include "TextRenderer.h"
#include "HUD.h"
#include "Entity.h"
#include "Button.h"

class PauseState : public State {
public:
	PauseState();
	~PauseState();
	void initialize();
	void update();
	void handleEvent();
	void pause();
	void play();
	void draw();

private:
	Entity m_entity;
	Button m_exitButton;
	Button m_resumeButton;
	// Camera m_camera;
	HUD m_hud;
	TextRenderer m_textRenderer;
};
