#pragma once
#include "State.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
#include "ParticleSystem.h"
#include "Timer.h"
#include "Camera.h"
#include "Sprite2D.h"

class EndRoundState : public State {
public:
	EndRoundState();
	~EndRoundState();
	void initialize();
	void update();
	void handleEvent();
	void pause();
	void play();
	void draw();
	void setParticleColorByPrize(size_t prize);
	void setVictoryText(string text);
	void setVictoryColor(float4 color);
	void setBowlMaterial(size_t contentIndex, int bowlMaterial);
	void setTimeText(string text);
	void setConfettiPower(float emitRate);

private:
	Camera m_camera;
	Timer m_timer;
	Sprite2D m_background;
	TextRenderer m_textRenderer;
	string m_victoryText;
	float4 m_victoryColor;
	string m_timeText;
	Button m_exitButton;
	Button m_levelSelectButton;
	Button m_restartButton;
	ParticleSystem m_particleSystem;
	Entity m_bowlContents[3];
	Entity m_bowl;
	size_t m_currentBowlContent;


	Entity m_entity;
};
