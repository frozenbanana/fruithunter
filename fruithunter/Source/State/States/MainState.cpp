#include "MainState.h"
#include "Renderer.h"
#include "Settings.h"
#include "AudioController.h"
#include "SaveManager.h"

void MainState::setButtons_menu() {
	string buttonTexts[btn_length] = { "Start", "Settings", "Exit", "Editor" };
	float2 btn_pos_start(200, 400);
	float btn_stride_y = 85;
	float btn_delay_stride = 0.1f;
	for (size_t i = 0; i < btn_length; i++) {
		m_btn_menu_buttons[i].set(
			btn_pos_start + float2(0, btn_stride_y) * i, buttonTexts[i], btn_delay_stride * i);
	}
	m_btn_menu_credits.set(float2(1280 - 150, 720 - 75), "Credits", btn_length * btn_delay_stride);
}

void MainState::setButtons_levelSelect() {
	m_btn_levelSelect_back.set(float2(1280 - 150, 720 - 75), "Back", 0);
	m_btn_levelSelect_hunt.set(float2(1280.f/2, 720 - 75), "Hunt", 0.2f);
	m_btn_levelSelect_controls.set(float2(150, 720 - 75), "Controls", 0.4f);
}

void MainState::setButtons_credits() { m_btn_credits_back.set(float2(150, 720 - 75), "Back", 0); }

void MainState::changeToLevel(size_t levelIndex) {
	AudioController::getInstance()->flush();
	SceneManager::getScene()->load("scene" + to_string(levelIndex));
	push(State::PlayState);
}

void MainState::changeMainState(MainStateType state) {
	m_stateSwitching = true;
	m_stateTarget = state;
	if (m_mainState != Menu)
		setButtons_menu();
	if (m_mainState != LevelSelect)
		setButtons_levelSelect();
	if (m_mainState != Credits)
		setButtons_credits();
}

void MainState::draw_ui_menu(float alpha) {
	// Logo
	for (size_t i = 0; i < m_letters.size(); i++) {
		m_letters[i].letter.setAlpha(alpha);
		m_letters[i].letter.draw();
	}

	// Draw menu buttons
	m_btn_menu_buttons[btn_start].setAlpha(alpha);
	m_btn_menu_buttons[btn_start].draw();
	m_btn_menu_buttons[btn_settings].setAlpha(alpha);
	m_btn_menu_buttons[btn_settings].draw();
	m_btn_menu_buttons[btn_exit].setAlpha(alpha);
	m_btn_menu_buttons[btn_exit].draw();
	m_btn_menu_credits.setAlpha(alpha);
	m_btn_menu_credits.draw();
	if (DEBUG) {
		m_btn_menu_buttons[btn_editor].setAlpha(alpha);
		m_btn_menu_buttons[btn_editor].draw();
	}
}

void MainState::draw_ui_levelselect(float alpha) {
	int selectedLevel = m_levelSelected;
	int levelHighlighted = m_levelHighlighted;

	Color stdColor = Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f);
	Color stdLockedColor = Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f) * 0.6f;
	Color stdDarkTextColor = stdColor * 0.3f;
	Color stdLightTextColor = Color(1, 1, 1);

	m_btn_levelSelect_back.setAlpha(alpha);
	m_btn_levelSelect_back.draw();

	m_btn_levelSelect_controls.setAlpha(alpha);
	m_btn_levelSelect_controls.draw();

	m_btn_levelSelect_hunt.setAlpha(alpha);
	m_btn_levelSelect_hunt.draw();

	// level items
	float2 levelItem_startPos = float2(50, 120);
	float2 levelItem_padding = float2(0, 10);
	for (size_t lvl = 0; lvl < 3; lvl++) {
		bool locked = (lvl != 0 && !m_levelSelections[lvl - 1].completed);

		LevelOption& level = m_levelSelections[lvl];
		// container
		m_spr_levelItem_container.setAlpha(alpha);
		m_spr_levelItem_container.setScale(0.6f);
		float2 levelItemSize = m_spr_levelItem_container.getSize();
		float2 itemPos =
			levelItem_startPos + (float2(0, levelItemSize.y) + levelItem_padding) * lvl;
		m_spr_levelItem_container.setPosition(itemPos);
		m_spr_levelItem_container.setColor(locked ? stdLockedColor : stdColor);
		m_spr_levelItem_container.draw();
		// medal
		float2 levelDotOffset = float2(26.5, 0);
		if (level.completed && level.grade != TimeTargets::NR_OF_TIME_TARGETS) {
			Sprite2D& spr_medal = m_spr_levelItem_medals[level.grade];
			spr_medal.setAlpha(alpha);
			spr_medal.setScale(0.6f);
			spr_medal.setPosition(itemPos + levelDotOffset);
			spr_medal.draw();
		}
		// level idx
		m_textRenderer_lato.setAlignment(); // center
		m_textRenderer_lato.setScale(0.25f);
		m_textRenderer_lato.setAlpha(alpha);
		m_textRenderer_lato.setPosition(itemPos + levelDotOffset + float2(-3, 0));
		m_textRenderer_lato.setText(to_string(lvl));
		m_textRenderer_lato.setColor(stdDarkTextColor);
		m_textRenderer_lato.draw();
		// name
		m_textRenderer_lato.setAlignment(HorizontalAlignment::AlignLeft); // center
		m_textRenderer_lato.setScale(0.25f);
		m_textRenderer_lato.setAlpha(alpha);
		m_textRenderer_lato.setPosition(itemPos + float2(55, 0));
		m_textRenderer_lato.setText(locked ? "Locked" : level.name);
		if (levelHighlighted == lvl)
			m_textRenderer_lato.setColor(Color(1, 1, 1));
		else
			m_textRenderer_lato.setColor(stdDarkTextColor);
		m_textRenderer_lato.draw();
		// marker
		if (selectedLevel == lvl) {
			m_spr_levelItem_marker.setAlpha(alpha);
			m_spr_levelItem_marker.setScale(0.6f);
			float animFactor = sin(m_timer.getTimePassed() * 2 * XM_PI / 2.f) * 0.5f + 0.5f;
			m_spr_levelItem_marker.setPosition(
				itemPos + float2(levelItemSize.x + 0, 0) + float2(10, 0) * animFactor);
			m_spr_levelItem_marker.setColor(stdColor);
			m_spr_levelItem_marker.draw();
		}
	}

	// level info
	{
		LevelOption& level = m_levelSelections[selectedLevel];
		// container
		float2 lvlInfoPos = float2(1280.f / 2, 720.f / 2 + 70);
		m_spr_levelInfo_container.setAlpha(alpha);
		m_spr_levelInfo_container.setScale(0.5f);
		float2 lvlInfoSize = m_spr_levelInfo_container.getSize();
		m_spr_levelInfo_container.setPosition(lvlInfoPos);
		m_spr_levelInfo_container.setColor(stdColor);
		m_spr_levelInfo_container.draw();
		// header
		m_textRenderer_lato.setAlignment(); // center
		m_textRenderer_lato.setScale(0.25f);
		m_textRenderer_lato.setAlpha(alpha);
		m_textRenderer_lato.setPosition(lvlInfoPos + float2(0, -lvlInfoSize.y / 2 + 26));
		m_textRenderer_lato.setText(level.name);
		m_textRenderer_lato.setColor(stdDarkTextColor);
		m_textRenderer_lato.draw();
		// best time
		m_textRenderer_lato.setAlignment(HorizontalAlignment::AlignLeft); // center
		m_textRenderer_lato.setScale(0.25f);
		m_textRenderer_lato.setAlpha(alpha);
		m_textRenderer_lato.setPosition(lvlInfoPos + float2(-lvlInfoSize.x / 2 + 30, -62));
		string timeStr = "--:--.---";
		if (level.grade != NR_OF_TIME_TARGETS)
			timeStr = Milliseconds2DisplayableString(level.timeMs);
		m_textRenderer_lato.setText("Best: " + timeStr);
		m_textRenderer_lato.setColor(stdLightTextColor);
		m_textRenderer_lato.draw();
		// medals
		float2 coin_startPos = lvlInfoPos + float2(-lvlInfoSize.x / 2 + 50, -15);
		float2 coin_padding = float2(0, 7);
		for (size_t c = 0; c < TimeTargets::NR_OF_TIME_TARGETS; c++) {
			m_medalSprites[c].setScale(0.05f);
			float2 coinSize = m_medalSprites[c].getSize();
			float2 coinPos = coin_startPos + (float2(0, coinSize.y) + coin_padding) * c;
			if (level.completed && level.grade <= c)
				m_medalSprites[c].setColor(float4(1, 1, 1, 1));
			else
				m_medalSprites[c].setColor(float4(0.25, 0.25, 0.25, 1));
			m_medalSprites[c].setAlpha(alpha);
			m_medalSprites[c].setPosition(coinPos);
			m_medalSprites[c].draw();
			// level time
			m_textRenderer_lato.setAlignment(HorizontalAlignment::AlignLeft);
			m_textRenderer_lato.setAlpha(alpha);
			m_textRenderer_lato.setScale(0.23f);
			m_textRenderer_lato.setPosition(coinPos + float2(coinSize.x/2 + 10, 0));
			m_textRenderer_lato.setText(
				Seconds2DisplayableString(
					m_levelData[selectedLevel].m_utility.timeTargets[c] / 1000) +
				" min");
			m_textRenderer_lato.setColor(stdLightTextColor);
			m_textRenderer_lato.draw();
		}
	}
	// Leaderboard
	{
		// container
		float2 boardPos = float2(1280.f * 0.82f, 720.f / 2 - 30);
		m_spr_leaderboard_container.setAlpha(alpha);
		m_spr_leaderboard_container.setScale(0.5f);
		float2 boardSize = m_spr_leaderboard_container.getSize();
		m_spr_leaderboard_container.setPosition(boardPos);
		m_spr_leaderboard_container.setColor(stdColor);
		m_spr_leaderboard_container.draw();
		// header
		m_textRenderer_lato.setAlignment(); // center
		m_textRenderer_lato.setScale(0.25f);
		m_textRenderer_lato.setAlpha(alpha);
		m_textRenderer_lato.setPosition(boardPos + float2(0, -boardSize.y / 2 + 26));
		m_textRenderer_lato.setText("Leaderboard");
		m_textRenderer_lato.setColor(stdDarkTextColor);
		m_textRenderer_lato.draw();
		// mini buttons
		Color iconColor = Color(1.f, 204 / 255.f, 0, 1);
		float2 minibtnCrown_offset = float2(-160, -150);
		m_spr_iconButton.setAlpha(alpha);
		m_spr_iconButton.setScale(0.5f);
		m_spr_iconButton.setPosition(boardPos + minibtnCrown_offset);
		m_spr_iconButton.setColor(stdColor);
		m_spr_iconButton.draw();

		m_spr_icon_crown.setAlpha(alpha);
		m_spr_icon_crown.setScale(0.5f);
		m_spr_icon_crown.setPosition(boardPos + minibtnCrown_offset);
		m_spr_icon_crown.setColor(iconColor);
		m_spr_icon_crown.draw();

		float2 minibtnMan_offset = float2(-160 + 70, -150);
		m_spr_iconButton.setAlpha(alpha);
		m_spr_iconButton.setScale(0.5f);
		m_spr_iconButton.setPosition(boardPos + minibtnMan_offset);
		m_spr_iconButton.setColor(stdColor);
		m_spr_iconButton.draw();

		m_spr_icon_man.setAlpha(alpha);
		m_spr_icon_man.setScale(0.5f);
		m_spr_icon_man.setPosition(boardPos + minibtnMan_offset);
		m_spr_icon_man.setColor(iconColor);
		m_spr_icon_man.draw();
		// entries
		float2 entry_startPos = boardPos + float2(-185, -100);
		float2 entry_offset = float2(0, 31);
		for (size_t i = 0; i < m_leaderboard.getEntryCount(); i++) {
			float2 entryPos = entry_startPos + entry_offset * i;
			LeaderboardEntry_t entry;
			if (m_leaderboard.getEntry(i, entry)) {
				m_textRenderer_lato.setScale(0.235f);
				m_textRenderer_lato.setAlpha(alpha);
				m_textRenderer_lato.setColor(stdLightTextColor);
				string rank = "";
				int missingDigits = 2 - (int)log10((float)entry.m_nGlobalRank);
				for (size_t d = 0; d < missingDigits; d++)
					rank += '0';
				rank += to_string(entry.m_nGlobalRank);
				string steam_name =
					string(SteamFriends()->GetFriendPersonaName(entry.m_steamIDUser));
				string scoreStr = Milliseconds2DisplayableString(entry.m_nScore);
				// rank
				m_textRenderer_lato.setAlignment(HorizontalAlignment::AlignLeft); // center
				m_textRenderer_lato.setText("[" + rank + "]");
				m_textRenderer_lato.setPosition(entryPos);
				m_textRenderer_lato.draw();
				float2 rankSize = m_textRenderer_lato.getSize();
				// name
				m_textRenderer_lato.setAlignment(HorizontalAlignment::AlignLeft); // center
				m_textRenderer_lato.setText(steam_name);
				const int maxNameWidth = 165;
				if (m_textRenderer_lato.getSize().x > maxNameWidth) {
					// limit steam name
					size_t idx = 0;
					string shortName = "";
					do {
						shortName += steam_name[idx++];
						m_textRenderer_lato.setText(shortName);
					} while (idx < steam_name.length() &&
							 m_textRenderer_lato.getSize().x < maxNameWidth);
					shortName += "...";
					m_textRenderer_lato.setText(shortName);
				}
				m_textRenderer_lato.setPosition(entryPos + float2(70, 0));
				m_textRenderer_lato.draw();
				// score
				m_textRenderer_lato.setAlignment(HorizontalAlignment::AlignRight); // center
				m_textRenderer_lato.setText(scoreStr);
				m_textRenderer_lato.setPosition(entryPos + float2(365, 0));
				m_textRenderer_lato.draw();
			}
		}
	}
}

void MainState::draw_ui_credits(float alpha) {
	m_btn_credits_back.setAlpha(alpha);
	m_btn_credits_back.draw();
}

MainState::MainState() : StateItem(StateItem::State::MainState) {}

MainState::~MainState() {}

void MainState::init() {
	m_sceneManager.setPlayerState(false);

	m_textRenderer_lato.setFont("lato.spritefont");

	m_levelItem_background.load("back_level.png");
	m_levelItem_background.setColor(Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f));
	m_levelItem_background.setAlignment(); // center
	m_levelItem_background.setScale(0.7f);

	string medalSpriteNames[TimeTargets::NR_OF_TIME_TARGETS] = { "coin_gold.png", "coin_silver.png",
		"coin_bronze.png" };
	for (size_t i = 0; i < TimeTargets::NR_OF_TIME_TARGETS; i++) {
		m_medalSprites[i].load(medalSpriteNames[i]);
		m_medalSprites[i].setScale(0.03f);
		m_medalSprites[i].setAlignment();
	}

	m_img_keylock.load("keylock.png");
	m_img_keylock.setAlignment(); // center

	m_ps_selected.load("levelselect selection", 30);
	m_ps_selected.setScale(float3(0.6f, 0.3f, 0.6f));

	m_letters.resize(11);
	string logoPaths[11] = {
		"fruithunter_logo_F_color.png",
		"fruithunter_logo_r_color.png",
		"fruithunter_logo_u_color.png",
		"fruithunter_logo_i_color.png",
		"fruithunter_logo_t_color.png",
		"fruithunter_logo_H_color.png",
		"fruithunter_logo_u_color.png",
		"fruithunter_logo_n_color.png",
		"fruithunter_logo_t_color.png",
		"fruithunter_logo_e_color.png",
		"fruithunter_logo_r_color.png",
	};
	for (size_t i = 0; i < m_letters.size(); i++) {
		m_letters[i].letter.load(logoPaths[i]);
		m_letters[i].letter.setScale(0.25f);
		m_letters[i].speedOffset = float2(RandomFloat(-0.15f, 0.15f), RandomFloat(-0.5f, 0.5f));
		m_letters[i].letter.setAlignment(); // center
	}

	m_spr_levelItem_container.load("level_holder.png");
	m_spr_levelItem_container.setAlignment(
		HorizontalAlignment::AlignLeft, VerticalAlignment::AlignCenter);
	m_spr_levelItem_marker.load("level_marker.png");
	m_spr_levelItem_marker.setAlignment();
	m_spr_levelItem_medals[TimeTargets::GOLD].load("dot_gold.png");
	m_spr_levelItem_medals[TimeTargets::GOLD].setAlignment();
	m_spr_levelItem_medals[TimeTargets::SILVER].load("dot_silver.png");
	m_spr_levelItem_medals[TimeTargets::SILVER].setAlignment();
	m_spr_levelItem_medals[TimeTargets::BRONZE].load("dot_bronze.png");
	m_spr_levelItem_medals[TimeTargets::BRONZE].setAlignment();
	m_spr_levelInfo_container.load("levelInfo.png");
	m_spr_levelInfo_container.setAlignment();
	m_spr_leaderboard_container.load("leaderboard_holder.png");
	m_spr_leaderboard_container.setAlignment();
	m_spr_icon_crown.load("icon_crown.png");
	m_spr_icon_crown.setAlignment();
	m_spr_icon_man.load("icon_man.png");
	m_spr_icon_man.setAlignment();
	m_spr_iconButton.load("iconbtn.png");
	m_spr_iconButton.setAlignment();
}

void MainState::update() {
	Input* ip = Input::getInstance();
	ip->setMouseModeAbsolute();

	float2 mousePos = ip->mouseXY();

	float3 treePos(56.4f, 9.5f, 18.2f);
	float3 bowPos = treePos + float3(10, 1.5, 5);

	m_timer.update();
	float dt = (float)m_timer.getDt();
	m_totalDelta = fmod((m_totalDelta + dt), (2.f * XM_PI));
	m_totalDelta_forBow += dt;

	// update scene
	m_sceneManager.update();

	// update precoded bow behavior
	float3 target = treePos + float3(0, 1.0, 0) +
					float3(RandomFloat(-1, 1), RandomFloat(-1, 1), RandomFloat(-1, 1)) * 1.5;
	float3 bowForward = target - bowPos;
	bowForward.Normalize();
	float3 rot = vector2Rotation(bowForward);
	m_bow.setOrientation(bowPos, rot);
	shared_ptr<Arrow> arrow;
	if (m_totalDelta_forBow >= m_shootDelay + m_bowHoldTime) {
		m_totalDelta_forBow = 0; // reset timer
								 // randomize bow values
								 // m_bowHoldTime = RandomFloat(1.0, 1.5);
	}
	else if (m_totalDelta_forBow >= m_bowHoldTime) {
		arrow = m_bow.update(dt, false); // release string
	}
	else {
		arrow = m_bow.update(dt, true); // pull string
	}
	if (arrow.get() != nullptr)
		SceneManager::getScene()->addArrow(arrow); // add shot arrow to array

	// update apple behavior (run around tree)
	float fruitAnimationCycle = 1 / 4.f + 1 / 5.f + 1 / 2.0f + 1 / 1.9f + 1 / 4.f +
								1 / 2.f; // apple frame speeds, added together
	Fruit* fruit = m_apple.get();
	float3 fruitPosition = treePos; // center fruit on tree
	fruitPosition +=
		float3(cos(m_totalDelta) * 2.f, 0.4f, sin(m_totalDelta) * 2.f); // walk around center
	fruitPosition.y += abs(sin(m_totalDelta * 4.f - 0.5f)) * 0.5f;		// fruit jump
	fruit->setPosition(fruitPosition);
	fruit->updateAnimated(8 * dt * fruitAnimationCycle / (2.f * XM_PI));
	fruit->setRotation(float3(0.0f, -m_totalDelta, 0.0f));

	if (!m_stateSwitching) {
		if (m_mainState == Menu) {
			// Logo update
			float offsetX = 1280.f / 16.f;
			float offsetY = 720.f / 6.0f;
			float t = (float)m_timer.getTimePassed();
			for (size_t i = 0; i < m_letters.size(); i++) {
				float2 movement = float2(sin(t + m_letters[i].speedOffset.x),
									  cos(t + m_letters[i].speedOffset.y)) *
								  10.f;
				m_letters[i].letter.setPosition(float2(offsetX, offsetY) + movement);
				offsetX += m_letters[i].letter.getTextureSize().x / (1.65f * 2.f);
			}

			if (m_btn_menu_buttons[btn_start].update_behavior(dt)) {
				// start
				changeMainState(LevelSelect);
			}
			if (m_btn_menu_buttons[btn_settings].update_behavior(dt)) {
				// settings
				push(State::SettingState);
			}
			if (m_btn_menu_buttons[btn_exit].update_behavior(dt)) {
				// exit
				pop(false);
			}
			if (DEBUG && m_btn_menu_buttons[btn_editor].update_behavior(dt)) {
				// editor
				AudioController::getInstance()->flush();
				push(State::EditorState);
			}
			if (m_btn_menu_credits.update_behavior(dt)) {
				// open credits
				changeMainState(Credits);
			}
		}
		else if (m_mainState == LevelSelect) {
			if (m_btn_levelSelect_back.update_behavior(dt)) {
				// back to menu
				changeMainState(Menu);
			}
			if (m_btn_levelSelect_controls.update_behavior(dt)) {
				push(State::ControlState);
			}
			if (m_btn_levelSelect_hunt.update_behavior(dt)) {
				changeToLevel(m_levelSelected);
			}

			m_levelHighlighted = -1;
			static bool initDownload = false;
			float2 levelItem_startPos = float2(50, 120);
			float2 levelItem_padding = float2(0, 10);
			for (size_t lvl = 0; lvl < 3; lvl++) {
				LevelOption& level = m_levelSelections[lvl];
				bool locked = (lvl != 0 && !m_levelSelections[lvl - 1].completed);
				if (!locked) {
					float2 levelItemSize = m_spr_levelItem_container.getSize();
					float2 itemPos =
						levelItem_startPos + (float2(0, levelItemSize.y) + levelItem_padding) * lvl;
					m_spr_levelItem_container.setPosition(itemPos);
					if (m_spr_levelItem_container.getBoundingBox().isInside(mousePos)) {
						// hover
						m_levelHighlighted = lvl;
						if (ip->mousePressed(Input::MouseButton::LEFT)) {
							m_levelSelected = lvl;
							initDownload = false;
							m_leaderboard.FindLeaderboard(
								m_levelData[lvl].m_leaderboardName.c_str());
						}
					}
				}
			}
			if (m_leaderboard.getRequestState_FindLeaderboard() ==
					CSteamLeaderboard::RequestState::r_finished &&
				!initDownload) {
				m_leaderboard.DownloadScores(
					ELeaderboardDataRequest::k_ELeaderboardDataRequestGlobal, 0, 10);
				initDownload = true;
			}

			// update level frames
			float itemWidthOffset = 325;
			float totalItemWidth = itemWidthOffset * (3 - 1);
			for (size_t i = 0; i < 3; i++) {
				// update frame position
				float2 desiredItemPos =
					float2(1280.f / 2 + itemWidthOffset * i - totalItemWidth * 0.5f, 720 - 250);
				if (m_levelHighlighted == i)
					desiredItemPos += float2(0, -25);
				float2 currentItemPos = m_levelSelections[i].position_hud;
				float lerp = 1 - pow(m_levelSelections[i].catchup, dt);
				m_levelSelections[i].position_hud +=
					(desiredItemPos - currentItemPos) * Clamp<float>(lerp, 0, 1);

				// hover and click behavior
				if (i == 0 || m_levelSelections[i - 1].completed) {
					// valid level to process
					m_levelItem_background.setPosition(m_levelSelections[i].position_hud);
					float2 mp = ip->mouseXY();
					if (m_levelItem_background.getBoundingBox().isInside(mp)) {
						// hovering frame
						m_levelHighlighted = (int)i;
						if (ip->mousePressed(Input::LEFT)) {
							// clicked frame
							changeToLevel(m_levelHighlighted);
						}
					}
				}
			}
		}
		else if (m_mainState == Credits) {
			if (m_btn_credits_back.update_behavior(dt))
				changeMainState(Menu);
		}
	}
	else {
		// update camera movement
		// move towards menu state
		m_stateSwitchFactor =
			Clamp<float>(m_stateSwitchFactor + dt * (1.f / m_stateSwitchTime), 0, 1);

		// set cam transformation
		float c = (1 - cos(m_stateSwitchFactor * XM_PI)) * 0.5f;
		CamTransformState source = m_camTransformStates[m_mainState];
		CamTransformState dest = m_camTransformStates[m_stateTarget];

		m_sceneManager.getScene()->m_camera.setEye(source.position * (1 - c) + dest.position * c);
		float3 sourceForwardPoint = Normalize(source.target - source.position) + source.position;
		float3 destForwardPoint = Normalize(dest.target - dest.position) + dest.position;
		m_sceneManager.getScene()->m_camera.setTarget(
			sourceForwardPoint * (1 - c) + destForwardPoint * c);

		if (m_stateSwitchFactor == 1) {
			// switch state to target
			m_stateSwitching = false;
			m_stateSwitchFactor = 0;
			m_mainState = m_stateTarget;
		}
	}

	// update level selection particlesystem effect
	m_ps_selected.setPosition(
		m_levelSelections[m_levelHighlighted].obj_bowl.getPosition() + float3(0, 0.2f, 0));
	m_ps_selected.update(dt);
}

void MainState::draw() {
	//	__SHADOWS__
	m_sceneManager.setup_shadow();
	// custom shadow drawing
	m_apple->draw_animate_onlyMesh();
	m_obj_creditsSign.draw_onlyMesh(float3());
	// standard shadow drawing
	m_sceneManager.draw_shadow();

	//	__COLOR__
	m_sceneManager.setup_color();
	// custom drawing (with darkoutlines)
	for (size_t i = 0; i < 3; i++) {
		if (i == 0 || m_levelSelections[i - 1].completed) {
			float3 highlightColor = float3(1.f) * (m_levelHighlighted == i ? 1.f : 0.3f);
			m_levelSelections[i].obj_bowl.draw(highlightColor);
			if (m_levelSelections[i].completed)
				m_levelSelections[i].obj_content.draw(highlightColor);
		}
	}
	m_bow.draw();
	// standard drawing
	m_sceneManager.draw_color();
	// custom drawing (without dark outline)
	Renderer::getInstance()->setBlendState_NonPremultiplied();
	m_apple->draw_animate();
	m_obj_creditsSign.draw(float3(1.5f));
	Renderer::getInstance()->setBlendState_Opaque();
	m_ps_selected.draw();

	/* -- MENU UI -- */

	// game version
	m_textRenderer.setAlignment(
		HorizontalAlignment::AlignRight, VerticalAlignment::AlignBottom); // center
	m_textRenderer.setScale(0.4f);
	m_textRenderer.setAlpha(1);
	m_textRenderer.setPosition(float2(1280, 720) * 0.99f);
	string gameVersionStr = "v" + to_string(GAME_VERSION) + "." + to_string(GAME_PATCH);
	m_textRenderer.setText(gameVersionStr);
	m_textRenderer.setColor(Color(0, 0, 0, 1));
	m_textRenderer.draw();
	m_textRenderer.setColor(Color(1, 1, 1, 1));

	float source_alpha = Clamp<float>((1 - m_stateSwitchFactor) * 2 - 1, 0, 1);
	float dest_alpha = Clamp<float>(m_stateSwitchFactor * 2 - 1, 0, 1);
	float stateAlpha[NR_OF_STATES] = { 0, 0, 0 };
	stateAlpha[m_mainState] = source_alpha;
	if (m_stateSwitching)
		stateAlpha[m_stateTarget] = dest_alpha;

	// menu
	draw_ui_menu(stateAlpha[Menu]);

	// level select
	draw_ui_levelselect(stateAlpha[LevelSelect]);

	// Credits
	draw_ui_credits(stateAlpha[Credits]);
}

void MainState::play() {
	if (SceneManager::getScene()->m_sceneName != "intro")
		m_sceneManager.load("intro");
	m_apple = make_shared<Apple>(float3(58.0f, 10.1f, 16.9f));
	m_timer.reset();

	// set camera position
	m_sceneManager.getScene()->m_camera.setView(m_camTransformStates[m_mainState].position,
		m_camTransformStates[m_mainState].target, float3(0.f, 1.f, 0.f));

	// menu music
	if (!AudioController::getInstance()->isListed(m_menuMusic))
		m_menuMusic = AudioController::getInstance()->play(
			"banana_sunrise", AudioController::SoundType::Music, true);

	// load player progression
	for (size_t i = 0; i < 3; i++) {
		m_levelData[i].load_raw("scene" + to_string(i));
	}

	// setup buttons
	setButtons_menu();
	setButtons_levelSelect();
	setButtons_credits();

	// levelselection setup
	float itemWidthOffset = 325;
	float totalItemWidth = itemWidthOffset * (3 - 1);

	float3 bowlPositions[3] = { float3(67.455f, 10.528f, 20.378f),
		float3(66.942f, 10.528f, 19.874f), float3(66.410f, 10.528f, 19.344f) };
	string bowlLevelContentObjName[3] = { "BowlContent1", "BowlContent2", "BowlContent3" };
	string bowlGradeObjName[TimeTargets::NR_OF_TIME_TARGETS + 1] = { "bowl_gold", "bowl_silver",
		"bowl_bronze", "bowl_bronze" };
	for (size_t i = 0; i < 3; i++) {
		string scene = "scene" + to_string(i);
		SceneAbstactContent sceneContent;
		sceneContent.load_raw(scene);

		time_t timeMs = 0;
		TimeTargets grade = TimeTargets::NR_OF_TIME_TARGETS;
		if (SaveManager::getInstance()->getLevelProgress(scene, timeMs)) {
			grade = SceneManager::getScene()->getTimeTargetGrade(
				timeMs, sceneContent.m_utility.timeTargets);
		}
		bool completed = (grade != TimeTargets::NR_OF_TIME_TARGETS);

		float3 position = bowlPositions[i];

		m_levelSelections[i].obj_bowl.load(bowlGradeObjName[grade]);
		m_levelSelections[i].obj_content.load(bowlLevelContentObjName[i]);
		m_levelSelections[i].obj_bowl.setPosition(position);
		m_levelSelections[i].obj_content.setPosition(position);
		float bowlScale = 0.4f;
		m_levelSelections[i].obj_bowl.setScale(bowlScale);
		m_levelSelections[i].obj_content.setScale(bowlScale);

		m_levelSelections[i].completed = completed;
		m_levelSelections[i].grade = grade;
		m_levelSelections[i].timeMs = timeMs;

		float2 desiredItemPos =
			float2(1280.f / 2 + itemWidthOffset * i - totalItemWidth * 0.5f, 720 - 250);
		m_levelSelections[i].position_hud = desiredItemPos;
	}
	m_levelSelections[0].name = "Beginners Salad";
	m_levelSelections[1].name = "Overnight Salad";
	m_levelSelections[2].name = "Hot Rainbow Salad";

	m_levelsAvailable = 1;
	for (size_t i = 1; i < 3; i++) {
		if (m_levelSelections[i - 1].completed)
			m_levelsAvailable++;
	}

	// Credits Setup
	m_obj_creditsSign.load("CreditsSign");
	float c = 1.2f;
	float3 pos = m_camTransformStates[MainStateType::Credits].position;
	float3 tar = m_camTransformStates[MainStateType::Credits].target;
	float3 dir = Normalize(tar - pos);
	float3 signPos = pos + dir * c;
	signPos.y = SceneManager::getScene()->m_terrains.getHeightFromPosition(signPos);
	m_obj_creditsSign.setPosition(signPos);
	m_obj_creditsSign.lookTo(dir * float3(1, 0, 1));
}

void MainState::pause() {}

void MainState::restart() {}
