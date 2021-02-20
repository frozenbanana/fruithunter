#pragma once
#include "StateItem.h"
#include "TextRenderer.h"
#include "Button.h"
#include "Timer.h"
#include "Apple.h"
#include "Bow.h"
#include "Sprite2D.h"
#include "SceneManager.h"
#include "Menu_PoppingButton.h"
#include "Menu_PoppingArrowButton.h"
#include "GlobalNamespaces.h"

class MainState : public StateItem {
private:
	TextRenderer m_textRenderer;
	Timer m_timer;

	// Scene variables
	Camera m_camera;
	float m_cam_slider = 0;
	float3 m_cam_pos_menu = float3(58.0f, 10.9f, 21.9f);
	float3 m_cam_pos_levelSelect = float3(65.753, 9.530 + 1.25, 20.849);
	float3 m_cam_target_menu = float3(61.3f, 10.1f, -36.0f);
	float3 m_cam_target_levelSelect =
		float3(66.960, 9.530-0.35, 19.784);
	float m_totalDelta = 0.f;
	shared_ptr<Apple> m_apple;

	string m_levelSelect_header = "Select Hunt";
	int m_levelHighlighted = 0;
	ParticleSystem m_ps_selected;
	Sprite2D m_levelItem_background;
	Sprite2D m_medalSprites[TimeTargets::NR_OF_TIME_TARGETS];
	Sprite2D m_img_keylock;
	struct LevelOption {
		// 3d object
		Entity obj_bowl, obj_content;
		// hud positioning
		float2 position_hud;
		float catchup = 0.01f;
		// description and info
		string name = "";
		bool completed = false;
		TimeTargets grade = TimeTargets::BRONZE;
	};
	LevelOption m_levelSelections[3];
	size_t m_levelsAvailable = 0;

	float m_totalDelta_forBow = 0.f;
	float m_bowHoldTime = 2;  // bow holds in x seconds before firing
	float m_shootDelay = 2.5; // wait x seconds after shot
	Bow m_bow;
	Menu_PoppingArrowButton m_selectionArrows[2]; // 0 = left, 1 = right

	SceneManager sceneManager;

	SceneAbstactContent m_levelData[3];

	enum MenuState {
		Menu = -1,
		LevelSelect = 1 
	} m_menuState = Menu;
	float m_stateSwitchTime = 1; // time in seconds to switch state

	// Logo
	struct LogoLetter {
		Sprite2D letter;
		float2 speedOffset;
	};
	vector<LogoLetter> m_letters;

	enum MenuButtons {
		btn_start,
		btn_settings,
		btn_exit,
		btn_editor,
		btn_length
	};
	Menu_PoppingButton m_btn_buttons[btn_length];
	Menu_PoppingButton m_btn_credits;
	Menu_PoppingButton m_btn_back;
	Menu_PoppingButton m_btn_play;

	SoundID m_menuMusic = 0;

	void setButtons_menu();
	void setButtons_levelSelect();

	static string asTimer(size_t seconds);

	void changeToLevel(size_t levelIndex);

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
