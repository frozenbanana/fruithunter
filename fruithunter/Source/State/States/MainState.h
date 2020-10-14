#pragma once
#include "StateItem.h"
#include "TextRenderer.h"
#include "Button.h"
#include "Timer.h"
#include "Apple.h"
#include "Bow.h"
#include "Sprite2D.h"
#include "SceneManager.h"
#include "UI_Button.h"

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

	struct ButtonOption : UI_Button {
		float delay = 0;
		float timer = 0;

		float scale_desired = 0;
		const float scale_desired_hovering = 0.9;
		const float scale_desired_standard = 0.75;

		float scale_velocity = 0;
		const float scale_spring_speed = 100;
		const float scale_spring_friction = 0.01;

		bool update_behavior(float dt);
		void set(float2 position = float2(), string text = "", float _delay = 0);
		ButtonOption();
	};

	enum MenuButtons {
		btn_start,
		btn_settings,
		btn_exit,
		btn_editor,
		btn_length
	};
	ButtonOption m_buttons[btn_length];

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
