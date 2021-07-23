#pragma once
#include "StateItem.h"
#include "Text2D.h"
#include "Button.h"
#include "Timer.h"
#include "Apple.h"
#include "Bow.h"
#include "Sprite2D.h"
#include "SceneManager.h"
#include "Menu_PoppingButton.h"
#include "Menu_PoppingArrowButton.h"
#include "UIContainer.h"
#include "Menu_LeaderboardToggle.h"
#include "GlobalNamespaces.h"
#include "CSteamLeaderboards.h"

class MainState : public StateItem {
private:
	ELeaderboardDataRequest m_leaderboardRequest =
		ELeaderboardDataRequest::k_ELeaderboardDataRequestGlobalAroundUser;
	CSteamLeaderboard m_leaderboard;
	Text2D m_textRenderer;
	Text2D m_textRenderer_lato;

	// Scene variables
	enum MainStateType {
		Menu,
		LevelSelect,
		Credits,
		NR_OF_STATES
	} m_mainState = Menu,
	  m_stateTarget = Menu;
	float m_stateSwitchTime = 1; // time in seconds to switch state
	float m_stateSwitchFactor = 0;
	bool m_stateSwitching = false;
	struct CamTransformState {
		float3 position, target;
		CamTransformState(float3 _position, float3 _target) {
			position = _position;
			target = _target;
		}
	};
	CamTransformState m_camTransformStates[NR_OF_STATES] = {
		CamTransformState(float3(58.0f, 10.9f, 21.9f), float3(61.3f, 10.1f, -36.0f)),
		CamTransformState(float3(65.81f, 11.32f + 0.2f, 21.01f),
			float3(65.81f, 11.32f + 0.2f, 21.01f) + float3(0.54f, -0.62f - 0.1f, -0.56f)),
		CamTransformState(float3(47.93f, 11.02f, 19.48f),
			float3(47.93f, 11.02f, 19.48f) + float3(0.7f, -0.07f, -0.71f))
	};

	float m_totalDelta = 0.f;
	shared_ptr<Apple> m_apple;

	string m_levelSelect_header = "Select Hunt";
	int m_levelHighlighted = 0, m_levelSelected = 0;
	ParticleSystem m_ps_selected;
	Sprite2D m_levelItem_background;
	Sprite2D m_medalSprites[TimeTargets::NR_OF_TIME_TARGETS];
	Sprite2D m_img_keylock;

	Sprite2D m_spr_levelItem_container;
	Sprite2D m_spr_levelItem_marker;
	Sprite2D m_spr_levelItem_medals[TimeTargets::NR_OF_TIME_TARGETS];
	Sprite2D m_spr_levelInfo_container;
	Sprite2D m_spr_leaderboard_container;
	Menu_LeaderboardToggle m_toggle_global = Menu_LeaderboardToggle("icon_crown.png");
	Menu_LeaderboardToggle m_toggle_personal = Menu_LeaderboardToggle("icon_man.png");

	struct LevelOption {
		// 3d object
		Entity obj_bowl, obj_content;
		// hud positioning
		float2 position_hud;
		float catchup = 0.01f;
		// description and info
		string name = "";
		bool completed = false;
		size_t timeMs = 0;
		TimeTargets grade = TimeTargets::BRONZE;
	};
	LevelOption m_levelSelections[3];
	size_t m_levelsAvailable = 0;

	float m_totalDelta_forBow = 0.f;
	float m_bowHoldTime = 2;  // bow holds in x seconds before firing
	float m_shootDelay = 2.5; // wait x seconds after shot
	Bow m_bow;

	Entity m_obj_creditsSign;

	SceneManager m_sceneManager;

	SceneAbstactContent m_levelData[3];

	// Logo
	struct LogoLetter {
		Sprite2D letter;
		float2 speedOffset;
	};
	vector<LogoLetter> m_letters;

	enum MenuButtons { btn_start, btn_settings, btn_exit, btn_editor, btn_length };
	Menu_PoppingButton m_btn_menu_buttons[btn_length];
	Menu_PoppingButton m_btn_menu_credits;
	Menu_PoppingButton m_btn_levelSelect_back;
	Menu_PoppingButton m_btn_levelSelect_controls;
	Menu_PoppingButton m_btn_levelSelect_hunt;
	Menu_PoppingButton m_btn_credits_back;

	//shared_ptr<Drawable2D> m_ui_selection;
	//bool m_ui_editorOpen = false;
	//UIContainer m_ui_mainContainer;
	//bool m_ui_edit_active = false;
	//float2 m_ui_edit_mouseOffset;

	SoundID m_menuMusic = 0;

	float m_totalTime = 0;

	void setButtons_menu();
	void setButtons_levelSelect();
	void setButtons_credits();

	void changeToLevel(size_t levelIndex);

	void changeMainState(MainStateType state);

	void draw_ui_menu(float alpha);
	void draw_ui_levelselect(float alpha);
	void draw_ui_credits(float alpha);

	void update_ui_menu(float dt);
	void update_ui_levelselect(float dt);
	void update_ui_credits(float dt);

	void downloadLeaderboardScore();

public:
	MainState();
	~MainState();

	void init() override;

	void update(double dt) override;
	void draw() override;

	void play() override;
	void pause() override;
	void restart() override;
};
