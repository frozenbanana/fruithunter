#pragma once
#include "StateItem.h"
#include "Text2D.h"
#include "Entity.h"
#include "Button.h"
#include "ParticleSystem.h"
#include "Timer.h"
#include "Camera.h"
#include "Sprite2D.h"
#include "Menu_PoppingButton.h"
#include "CSteamLeaderboards.h"

class EndRoundState : public StateItem {
private:
	Camera m_camera;
	Timer m_timer;
	Text2D m_textRenderer;
	string m_victoryText;
	float4 m_victoryColor;
	string m_timeText;
	ParticleSystem m_particleSystem;
	Entity m_bowlContent;
	Entity m_bowl;
	size_t m_currentBowlContent;
	Sprite2D m_spr_background;

	size_t m_leaderboard_score = 0;
	CSteamLeaderboard m_leaderboard;

	enum UploadState {
		Disabled,
		Waiting,
		Failed,
		Finished 
	} m_uploadState;

	Menu_PoppingButton m_btn_play;
	Menu_PoppingButton m_btn_back;

public:
	EndRoundState();
	~EndRoundState();

	void init();
	void update();
	void draw();

	void pause();
	void play();
	void restart();

	void setParticleColorByPrize(size_t prize);
	void setVictoryText(string text);
	void setVictoryColor(float4 color);
	void setBowl(string bowlContentEntityName, int bowlMaterial);
	void setTimeText(string text);
	void setConfettiPower(float emitRate);

};
