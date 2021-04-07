#include "MainState.h"
#include "Renderer.h"
#include "Settings.h"
#include "AudioController.h"
#include "SaveManager.h"

void MainState::setButtons_menu() {
	string buttonTexts[btn_length] = { "Start", "Settings", "Exit", "Editor" };
	float2 btn_pos_start(200, 400);
	float btn_stride_y = 85;
	float btn_delay_stride = 0.1;
	for (size_t i = 0; i < btn_length; i++) {
		m_btn_menu_buttons[i].set(
			btn_pos_start + float2(0, btn_stride_y) * i, buttonTexts[i], btn_delay_stride * i);
	}
	m_btn_menu_credits.set(float2(1280-150,720-75), "Credits", btn_length * btn_delay_stride);
}

void MainState::setButtons_levelSelect() {
	m_btn_levelSelect_back.set(float2(1280 - 150, 720 - 75), "Back", 0);
	m_btn_levelSelect_play.set(float2(1280 / 2, 720 - 75), "Play", 0.2);
}

void MainState::setButtons_credits() {
	m_btn_credits_back.set(float2(150, 720 - 75), "Back", 0);
}

string MainState::asTimer(size_t total) { 
	int minutes = total / 60;
	int seconds = total % 60;
	string str = (minutes < 10 ? "0" : "") + to_string(minutes) + ":" + (seconds < 10 ? "0" : "") +
				 to_string(seconds);
	return str;
}

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

MainState::MainState() : StateItem(StateItem::State::MainState) {}

MainState::~MainState() {}

void MainState::init() {
	m_sceneManager.setPlayerState(false);

	m_bow.setRecoveryTime(0);
	
	m_levelItem_background.load("back_level.png");
	m_levelItem_background.setColor(Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f));
	m_levelItem_background.setAlignment(); // center
	m_levelItem_background.setScale(0.7);

	string medalSpriteNames[TimeTargets::NR_OF_TIME_TARGETS] = { "coin_gold.png", "coin_silver.png",
		"coin_bronze.png" };
	for (size_t i = 0; i < TimeTargets::NR_OF_TIME_TARGETS; i++) {
		m_medalSprites[i].load(medalSpriteNames[i]);
		m_medalSprites[i].setScale(0.04f);
		m_medalSprites[i].setAlignment();
	}

	m_img_keylock.load("keylock.png");
	m_img_keylock.setAlignment();// center
	m_img_keylock.setScale(0.85);

	m_ps_selected.load(ParticleSystem::Type::LEVELSELECT_SELECTION, 30);
	m_ps_selected.setScale(float3(0.6, 0.3, 0.6));

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
}

void MainState::update() {
	Input* ip = Input::getInstance();
	ip->setMouseModeAbsolute();

	float3 treePos(56.4f, 9.5f, 18.2f);
	float3 bowPos = treePos + float3(10, 1.5, 5);

	m_timer.update();
	float dt = m_timer.getDt();
	m_totalDelta = fmod((m_totalDelta + dt), (2.f * XM_PI));
	m_totalDelta_forBow += dt;

	// update scene
	m_sceneManager.update();

	// update precoded bow behavior
	float3 target = treePos + float3(0, 1.0, 0) +
					float3(RandomFloat(-1, 1), RandomFloat(-1, 1), RandomFloat(-1, 1))*4;
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
		SceneManager::getScene()->addArrow(arrow);// add shot arrow to array

	// update apple behavior (run around tree)
	float fruitAnimationCycle = 1 / 4.f + 1 / 5.f + 1 / 2.0f + 1 / 1.9f + 1 / 4.f + 1 / 2.f; // apple frame speeds, added together
	Fruit* fruit = m_apple.get();
	float3 fruitPosition = treePos; // center fruit on tree
	fruitPosition += float3(cos(m_totalDelta)*2, 0.4, sin(m_totalDelta)*2); // walk around center
	fruitPosition.y += abs(sin(m_totalDelta * 4 - 0.5)) * 0.5; // fruit jump
	fruit->setPosition(fruitPosition);
	fruit->updateAnimated(8 * dt * fruitAnimationCycle / (2.f * XM_PI));
	fruit->setRotation(float3(0.0f, -m_totalDelta, 0.0f));

	if (!m_stateSwitching) {
		if (m_mainState == Menu) {
			// Logo update
			float offsetX = 1280.f / 16.f;
			float offsetY = 720.f / 6.0f;
			float t = m_timer.getTimePassed();
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
			if (m_btn_levelSelect_play.update_behavior(dt)) {
				// start level
				changeToLevel(m_levelHighlighted);
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
						m_levelHighlighted = i;
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
		m_levelSelections[m_levelHighlighted].obj_bowl.getPosition() + float3(0, 0.2, 0));
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
			float3 highlightColor = float3(1.) * (m_levelHighlighted == i ? 1 : 0.3);
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
	float source_alpha = Clamp<float>((1 - m_stateSwitchFactor) * 2 - 1, 0, 1);
	float dest_alpha = Clamp<float>(m_stateSwitchFactor * 2 - 1, 0, 1);
	float stateAlpha[NR_OF_STATES] = { 0, 0, 0 };
	stateAlpha[m_mainState] = source_alpha;
	if (m_stateSwitching)
		stateAlpha[m_stateTarget] = dest_alpha;

	// menu
	{
		float alpha = stateAlpha[Menu];
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

	// level select
	{
		float alpha = stateAlpha[LevelSelect];

		m_btn_levelSelect_back.setAlpha(alpha);
		m_btn_levelSelect_back.draw();

		m_btn_levelSelect_play.setAlpha(alpha);
		m_btn_levelSelect_play.draw();

		float itemWidthOffset = 325;
		float totalItemWidth = itemWidthOffset * (3 - 1);
		for (size_t i = 0; i < 3; i++) {
			float2 itemPos = m_levelSelections[i].position_hud;
			// background
			m_levelItem_background.setPosition(itemPos);
			m_levelItem_background.setAlpha(alpha);
			m_levelItem_background.draw();

			if (i == 0 || m_levelSelections[i - 1].completed) {
				// level index
				m_textRenderer.setAlignment(); // center
				m_textRenderer.setScale(0.4f);
				m_textRenderer.setAlpha(alpha);
				m_textRenderer.setPosition(itemPos + float2(115, 80));
				m_textRenderer.setText(to_string(i+1));
				m_textRenderer.draw();
				// text
				m_textRenderer.setAlignment(); // center
				m_textRenderer.setScale(0.25f);
				m_textRenderer.setAlpha(alpha);
				m_textRenderer.setPosition(itemPos + float2(0, -82));
				m_textRenderer.setText(m_levelSelections[i].name);
				m_textRenderer.draw();
				// grade
				float2 coinPos = itemPos + float2(-85, -30);
				for (int c = 0; c < TimeTargets::NR_OF_TIME_TARGETS; c++) {
					// coin medal
					float2 cur_coinPos = coinPos + float2(0, c * 45);
					if (m_levelSelections[i].completed && m_levelSelections[i].grade <= c)
						m_medalSprites[c].setColor(float4(1, 1, 1, 1));
					else
						m_medalSprites[c].setColor(float4(0.25, 0.25, 0.25, 1));
					m_medalSprites[c].setAlpha(alpha);
					m_medalSprites[c].setPosition(cur_coinPos);
					m_medalSprites[c].draw();
					// level time
					m_textRenderer.setAlignment(HorizontalAlignment::AlignLeft,
						VerticalAlignment::AlignCenter);
					m_textRenderer.setAlpha(alpha);
					m_textRenderer.setScale(0.25f);
					m_textRenderer.setPosition(cur_coinPos + float2(25, 0));
					m_textRenderer.setText(
						asTimer(m_levelData[i].m_utility.timeTargets[c]) + " min");
					m_textRenderer.draw();
				}
			}
			else {
				// level locked
				m_img_keylock.setPosition(itemPos);
				m_img_keylock.setAlpha(alpha);
				m_img_keylock.draw();
			}
		}
	}

	// Credits
	{ 
		float alpha = stateAlpha[Credits];

		m_btn_credits_back.setAlpha(alpha);
		m_btn_credits_back.draw();
	}
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
		m_levelData[i].load_raw("scene"+to_string(i));
	}

	// setup buttons
	setButtons_menu();
	setButtons_levelSelect();
	setButtons_credits();

	// levelselection setup
	float itemWidthOffset = 325;
	float totalItemWidth = itemWidthOffset * (3 - 1);

	float3 bowlPositions[3] = { 
		float3(67.455,10.528,20.378), 
		float3(66.942,10.528,19.874),
		float3(66.410,10.528,19.344) 
	};
	string bowlLevelContentObjName[3] = { "BowlContent1", "BowlContent2", "BowlContent3" };
	string bowlGradeObjName[TimeTargets::NR_OF_TIME_TARGETS] = { "bowl_gold", "bowl_silver",
		"bowl_bronze" };
	for (size_t i = 0; i < 3; i++) {
		const SceneCompletion* cp = SaveManager::getProgress("scene" + to_string(i));
		bool completed = false;
		TimeTargets grade = TimeTargets::BRONZE;
		if (cp) {
			completed = cp->isCompleted();
			grade = cp->grade;
		}
		float3 position = bowlPositions[i];

		m_levelSelections[i].obj_bowl.load(bowlGradeObjName[grade]);
		m_levelSelections[i].obj_content.load(bowlLevelContentObjName[i]);
		m_levelSelections[i].obj_bowl.setPosition(position);
		m_levelSelections[i].obj_content.setPosition(position);
		float bowlScale = 0.4;
		m_levelSelections[i].obj_bowl.setScale(bowlScale);
		m_levelSelections[i].obj_content.setScale(bowlScale);

		m_levelSelections[i].completed = completed;
		m_levelSelections[i].grade = grade;

		float2 desiredItemPos =
			float2(1280.f / 2 + itemWidthOffset * i - totalItemWidth * 0.5f, 720 - 250);
		m_levelSelections[i].position_hud = desiredItemPos;
	}
	m_levelSelections[0].name = "Beginners Salad";
	m_levelSelections[1].name = "Overnight Salad";
	m_levelSelections[2].name = "Hot Rainbow Salad";

	m_levelsAvailable = 1;
	for (size_t i = 1; i < 3; i++) {
		if (m_levelSelections[i-1].completed)
			m_levelsAvailable++;
	}	

	// Credits Setup
	m_obj_creditsSign.load("CreditsSign");
	float c = 1.2;
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
