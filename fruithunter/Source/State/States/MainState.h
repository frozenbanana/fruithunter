#pragma once
#include "StateItem.h"
#include "TextRenderer.h"
#include "Button.h"
#include "Timer.h"
#include "Apple.h"
#include "Bow.h"
#include "Sprite2D.h"
#include "SceneManager.h"

class MainState : public StateItem {
private:
	TextRenderer m_textRenderer;
	Timer m_timer;
	Button m_startButton;
	Button m_settingsButton;
	Button m_exitButton;
	Button m_editorButton;

	// Scene variables
	Camera m_camera;
	float m_totalDelta = 0.f;
	shared_ptr<Apple> m_apple;

	float m_totalDelta_forBow = 0.f;
	float m_bowHoldTime = 2;  // bow holds in x seconds before firing
	float m_shootDelay = 2.5; // wait x seconds after shot
	Bow m_bow;
	vector<shared_ptr<Arrow>> m_arrows;

	SceneManager sceneManager;

	// Logo
	struct LogoLetter {
		Sprite2D letter;
		float2 speedOffset;
	};
	vector<LogoLetter> m_letters;

public:
	MainState();
	~MainState();

	void init();

	void update();
	void draw();

	void play();
	void pause();
	void restart();

};
