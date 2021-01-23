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
		m_buttons[i].set(
			btn_pos_start + float2(0, btn_stride_y) * i, buttonTexts[i], btn_delay_stride * i);
	}
}

void MainState::setButtons_levelSelect() {
	m_back.set(float2(1280-150, 75), "Back");

	float hor_edgeOffset = 100;
	float ver_edgeOffset = 150;
	m_selectionArrows[0].set(float2(hor_edgeOffset, 720 - ver_edgeOffset), "", 0.2,
		Menu_PoppingArrowButton::FacingDirection::Left);
	m_selectionArrows[1].set(float2(1280 - hor_edgeOffset, 720 - ver_edgeOffset), "", 0.4,
		Menu_PoppingArrowButton::FacingDirection::Right);
}

MainState::MainState() : StateItem(StateItem::State::MainState) {}

MainState::~MainState() {}

void MainState::init() {
	m_bow.setRecoveryTime(0);

	m_camera.setView(m_cam_pos_menu, m_cam_target_menu, float3(0.f, 1.f, 0.f));

	string miniSprites[FruitType::NR_OF_FRUITS] = { "apple.png", "banana.png", "melon.png",
		"dragonfruit.png" };
	for (size_t i = 0; i < FruitType::NR_OF_FRUITS; i++) {
		m_miniFruitSprites[i].load(miniSprites[i]);
		m_miniFruitSprites[i].setScale(0.05f);
		m_miniFruitSprites[i].setAlignment(); // center
	}
	
	m_levelItem_background.load("back_level.png");
	m_levelItem_background.setColor(Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f));
	m_levelItem_background.setAlignment(); // center
	m_levelItem_background.setScale(0.7);

	m_coinHolderSprite.load("coin_holder.png");
	m_coinHolderSprite.setAlignment();
	m_coinHolderSprite.setScale(0.075f);

	string medalSpriteNames[TimeTargets::NR_OF_TIME_TARGETS] = { "coin_gold.png", "coin_silver.png",
		"coin_bronze.png" };
	for (size_t i = 0; i < TimeTargets::NR_OF_TIME_TARGETS; i++) {
		m_medalSprites[i].load(medalSpriteNames[i]);
		m_medalSprites[i].setScale(0.075f);
		m_medalSprites[i].setAlignment();
	}

	float3 bowlPositions[3] = { float3(67.614, 9.530, 20.688), float3(66.927, 9.530, 19.881),
		float3(66.216, 9.530, 19.077) };
	string bowlLevelContentObjName[3] = {"BowlContent1","BowlContent2","BowlContent3"};
	string bowlGradeObjName[TimeTargets::NR_OF_TIME_TARGETS] = { "bowl_gold", "bowl_silver",
		"bowl_bronze" };
	for (size_t i = 0; i < 3; i++) {
		const SceneCompletion* cp = SaveManager::getProgress("scene"+to_string(i));
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
		m_levelSelections[i].obj_bowl.setScale(0.5);
		m_levelSelections[i].obj_content.setScale(0.5);

		m_levelSelections[i].completed = completed;
		m_levelSelections[i].grade = grade;
	}
	// level 1
	m_levelSelections[0].terrainTypes[AreaTag::Plains] = true;
	m_levelSelections[0].name = "Beginners Salad";
	m_levelSelections[0].collectionGoal[FruitType::APPLE] = 7;
	m_levelSelections[0].collectionGoal[FruitType::MELON] = 2;

	m_levelSelections[1].terrainTypes[AreaTag::Forest] = true;
	m_levelSelections[1].name = "Overnight Salad";
	m_levelSelections[1].collectionGoal[FruitType::APPLE] = 2;
	m_levelSelections[1].collectionGoal[FruitType::MELON] = 2;
	m_levelSelections[1].collectionGoal[FruitType::BANANA] = 2;

	m_levelSelections[2].terrainTypes[AreaTag::Desert] = true;
	m_levelSelections[2].terrainTypes[AreaTag::Volcano] = true;
	m_levelSelections[2].name = "Hot Rainbow Salad";
	m_levelSelections[2].collectionGoal[FruitType::APPLE] = 2;
	m_levelSelections[2].collectionGoal[FruitType::MELON] = 2;
	m_levelSelections[2].collectionGoal[FruitType::BANANA] = 2;
	m_levelSelections[2].collectionGoal[FruitType::DRAGON] = 2;

	m_ps_selected.load(ParticleSystem::Type::LEVELSELECT_SELECTION, 30);
	m_ps_selected.setScale(float3(0.6, 0.3, 0.6));

	for (size_t i = 0; i < btn_length; i++) {
		m_buttons[i].setStandardColor(Color(42.f/255.f, 165.f/255.f, 209.f/255.f));
		m_buttons[i].setHoveringColor(Color(1.f, 210.f/255.f, 0.f));

		m_buttons[i].setTextStandardColor(Color(1.f, 1.f, 1.f));
		m_buttons[i].setTextHoveringColor(Color(0.f, 0.f, 0.f));
	}
	m_back.setStandardColor(Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f));
	m_back.setHoveringColor(Color(1.f, 210.f / 255.f, 0.f));
	m_back.setTextStandardColor(Color(1.f, 1.f, 1.f));
	m_back.setTextHoveringColor(Color(0.f, 0.f, 0.f));

	for (size_t i = 0; i < 2; i++) {
		m_selectionArrows[i].setStandardColor(Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f));
		m_selectionArrows[i].setHoveringColor(Color(1.f, 210.f / 255.f, 0.f));

		m_selectionArrows[i].setTextStandardColor(Color(1.f, 1.f, 1.f));
		m_selectionArrows[i].setTextHoveringColor(Color(0.f, 0.f, 0.f));

	}

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
	sceneManager.update(&m_camera);

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
		m_arrows.push_back(arrow); // add shot arrow to array

	// arrow collision
	QuadTree<shared_ptr<Entity>>* entities = &SceneManager::getScene()->m_entities;
	for (size_t i = 0; i < m_arrows.size(); i++) {
		if (m_arrows[i]->isActive()) {
			// !! if arrow collides with anything, then the arrow handles the behavior !!
			// check collision with terrains and static entities
			m_arrows[i]->collide_terrainBatch(dt, sceneManager.getScene()->m_terrains);
			for (size_t j = 0; j < entities->size(); j++) {
				m_arrows[i]->collide_entity(dt, *(*entities)[j]);
			}
		}
		// update arrow
		m_arrows[i]->update(dt);
	}

	// update apple behavior (run around tree)
	float fruitAnimationCycle = 1 / 4.f + 1 / 5.f + 1 / 2.0f + 1 / 1.9f + 1 / 4.f + 1 / 2.f; // apple frame speeds, added together
	Fruit* fruit = m_apple.get();
	float3 fruitPosition = treePos; // center fruit on tree
	fruitPosition += float3(cos(m_totalDelta)*2, 0.4, sin(m_totalDelta)*2); // walk around center
	fruitPosition.y += abs(sin(m_totalDelta * 4 - 0.5)) * 0.5; // fruit jump
	fruit->setPosition(fruitPosition);
	fruit->updateAnimated(8 * dt * fruitAnimationCycle / (2.f * XM_PI));
	fruit->setRotation(float3(0.0f, -m_totalDelta, 0.0f));

	if (m_cam_slider == 0) {
		// Logo update
		float offsetX = 1280.f / 16.f;
		float offsetY = 720.f / 6.0f;
		float t = m_timer.getTimePassed();
		for (size_t i = 0; i < m_letters.size(); i++) {
			float2 movement =
				float2(sin(t + m_letters[i].speedOffset.x), cos(t + m_letters[i].speedOffset.y)) *
				10.f;
			m_letters[i].letter.setPosition(float2(offsetX, offsetY) + movement);
			offsetX += m_letters[i].letter.getTextureSize().x / (1.65f * 2.f);
		}

		if (m_buttons[btn_start].update_behavior(dt)) {
			// start
			//push(State::LevelSelectState);
			m_menuState = LevelSelect;
			setButtons_levelSelect(); // reset buttons and create the popping effect
		}
		if (m_buttons[btn_settings].update_behavior(dt)) {
			// settings
			push(State::SettingState);
		}
		if (m_buttons[btn_exit].update_behavior(dt)) {
			// exit
			pop(false);
		}
		if (DEBUG && m_buttons[btn_editor].update_behavior(dt)) {
			// editor
			push(State::EditorState);
		}
	}
	else if (m_cam_slider == 1) {
		if (m_back.update_behavior(dt)) {
			// back to menu
			m_menuState = Menu;
			setButtons_menu(); // reset buttons and create the popping effect
		}
		if (m_selectionArrows[0].update_behavior(dt) || ip->keyPressed(Keyboard::Left)) {
			// left selection arrow
			m_levelHighlighted = mod(m_levelHighlighted-1, 3);
		}
		if (m_selectionArrows[1].update_behavior(dt) || ip->keyPressed(Keyboard::Right)) {
			// right selection arrow
			m_levelHighlighted = mod(m_levelHighlighted+1, 3);
		}
	}

	// move towards menu state
	m_cam_slider = Clamp<float>(m_cam_slider + dt * (1.f/m_stateSwitchTime)*m_menuState, 0, 1);

	// camera movement between menu states
	float c = (1-cos(m_cam_slider*XM_PI))*0.5f;
	m_camera.setEye(m_cam_pos_menu * (1 - c) + m_cam_pos_levelSelect * c);
	float3 cam_forward_menu = Normalize(m_cam_target_menu-m_cam_pos_menu);
	float3 cam_forward_levelSelect = Normalize(m_cam_target_levelSelect-m_cam_pos_levelSelect);
	m_camera.setTarget((m_cam_pos_menu+cam_forward_menu) * (1 - c) + (m_cam_pos_levelSelect+cam_forward_levelSelect) * c);

	// level select bowl selection
	float3 ray_position = m_camera.getPosition(), ray_forward;
	float2 mpos =
		float2(1 - (float)ip->mouseX() / SCREEN_WIDTH, (float)ip->mouseY() / SCREEN_HEIGHT);
	ray_forward = m_camera.getMousePickVector(mpos) * 100;
	bool anyHighlights = false;
	for (size_t i = 0; i < 3; i++) {
		if (i == 0 || m_levelSelections[i - 1].completed) {
			// valid level to select
			if (m_levelSelections[i].obj_bowl.castRay(ray_position, ray_forward) != -1) {
				// hovering
				anyHighlights = true;
				m_levelHighlighted = i;
				if (ip->mousePressed(Input::LEFT)) {
					// clicked
					// change Level
					SceneManager::getScene()->load("scene" + to_string(i));
					push(State::PlayState);
				}
			}
		}
	}

	m_ps_selected.setPosition(
		m_levelSelections[m_levelHighlighted].obj_bowl.getPosition() + float3(0, 0.2, 0));
	m_ps_selected.update(dt);

}

void MainState::draw() {
	//	__SHADOWS__
	sceneManager.setup_shadow(&m_camera);
	// custom shadow drawing
	m_apple->draw_animate_onlyMesh();
	for (size_t i = 0; i < m_arrows.size(); i++)
		m_arrows[i]->draw_onlyMesh(float3(1.));
	// standard shadow drawing
	sceneManager.draw_shadow();

	//	__COLOR__
	sceneManager.setup_color(&m_camera);
	// custom drawing (with darkoutlines)
	Renderer::getInstance()->enableAlphaBlending();
	m_apple->draw_animate();
	Renderer::getInstance()->disableAlphaBlending();
	for (size_t i = 0; i < 3; i++) {
		if (i == 0 || m_levelSelections[i - 1].completed) {
			float3 highlightColor = float3(1.) * (m_levelHighlighted == i ? 1 : 0.3);
			m_levelSelections[i].obj_bowl.draw(highlightColor);
			if (m_levelSelections[i].completed)
				m_levelSelections[i].obj_content.draw(highlightColor);
		}
	}
	m_ps_selected.draw();
	m_bow.draw();
	for (size_t i = 0; i < m_arrows.size(); i++)
		m_arrows[i]->draw();
	// standard drawing
	sceneManager.draw_color(&m_camera);
	// custom drawing (without dark outline)
	for (size_t i = 0; i < m_arrows.size(); i++)
		m_arrows[i]->draw_trailEffect();

	float menuAlpha = Clamp<float>((1 - m_cam_slider) * 2 - 1, 0, 1);
	// Logo
	for (size_t i = 0; i < m_letters.size(); i++) {
		m_letters[i].letter.setAlpha(menuAlpha);
		m_letters[i].letter.draw();
	}

	// Draw menu buttons
	m_buttons[btn_start].setAlpha(menuAlpha);
	m_buttons[btn_start].draw();
	m_buttons[btn_settings].setAlpha(menuAlpha);
	m_buttons[btn_settings].draw();
	m_buttons[btn_exit].setAlpha(menuAlpha);
	m_buttons[btn_exit].draw();
	if (DEBUG) {
		m_buttons[btn_editor].setAlpha(menuAlpha);
		m_buttons[btn_editor].draw();
	}

	// level select
	float levelSelectAlpha = Clamp<float>(m_cam_slider * 2 - 1, 0, 1);
	m_textRenderer.setAlignment(TextRenderer::HorizontalAlignment::LEFT, TextRenderer::VerticalAlignment::TOP);
	m_textRenderer.setScale(1.5);
	m_textRenderer.setAlpha(levelSelectAlpha);
	m_textRenderer.draw(m_levelSelect_header, float2(50,50));

	m_back.setAlpha(levelSelectAlpha);
	m_back.draw();

	float itemWidthOffset = 325;
	float totalItemWidth = itemWidthOffset * (3-1);
	for (size_t i = 0; i < 3; i++) {
		float2 itemPos =
			float2(1280.f / 2 + itemWidthOffset * i - totalItemWidth * 0.5f, 720 - 150);
		if (m_levelHighlighted == i)
			itemPos += float2(0,-25);
		// background
		m_levelItem_background.setPosition(itemPos);
		m_levelItem_background.setAlpha(levelSelectAlpha);
		m_levelItem_background.draw();
		// text
		m_textRenderer.setAlignment(
			TextRenderer::HorizontalAlignment::LEFT, TextRenderer::VerticalAlignment::TOP);
		m_textRenderer.setScale(0.25f);
		m_textRenderer.setAlpha(levelSelectAlpha);
		m_textRenderer.draw(m_levelSelections[i].name, itemPos+float2(-100,-70));
		// terrain types
		// collection goal
		float2 backSize = m_levelItem_background.getSize();
		float miniWidth = 100;
		for (size_t j = 0; j < FruitType::NR_OF_FRUITS; j++) {
			m_miniFruitSprites[j].setPosition(
				float2(-45, 60) + itemPos +
				float2((miniWidth / (FruitType::NR_OF_FRUITS - 1)) * j - miniWidth / 2, 0));
			m_miniFruitSprites[j].setAlpha(levelSelectAlpha);
			m_miniFruitSprites[j].draw();
		}
		// grade
		float2 coinPos = itemPos + float2(75, 50);
		m_coinHolderSprite.setPosition(coinPos);
		m_coinHolderSprite.setAlpha(levelSelectAlpha);
		m_coinHolderSprite.draw();
		if (m_levelSelections[i].completed) {
			TimeTargets grade = m_levelSelections[i].grade;
			m_medalSprites[grade].setPosition(coinPos);
			m_medalSprites[grade].setAlpha(levelSelectAlpha);
			m_medalSprites[grade].draw();
		}
	}

	for (size_t i = 0; i < 2; i++) {
		m_selectionArrows[i].setAlpha(levelSelectAlpha);
		m_selectionArrows[i].draw();
	}
}

void MainState::play() {
	sceneManager.load("intro");
	AudioController::getInstance()->play("ocean-music", AudioController::SoundType::Music);
	m_apple = make_shared<Apple>(float3(58.0f, 10.1f, 16.9f));
	m_arrows.clear();
	m_timer.reset();

	// setup buttons
	setButtons_menu();
	setButtons_levelSelect();
		
}

void MainState::pause() {}

void MainState::restart() {}
