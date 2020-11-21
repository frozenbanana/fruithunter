#include "MainState.h"
#include "Renderer.h"
#include "Settings.h"
#include "AudioController.h"

MainState::MainState() : StateItem(StateItem::State::MainState) { }

MainState::~MainState() {}

void MainState::init() {
	m_bow.setRecoveryTime(0);

	m_camera.setView(
		float3(58.0f, 10.9f, 21.9f), float3(61.3f, 10.1f, -36.0f), float3(0.f, 1.f, 0.f));

	for (size_t i = 0; i < btn_length; i++) {
		m_buttons[i].setStandardColor(Color(0.f, 0.627f, 0.647f));
		m_buttons[i].setHoveringColor(Color(1.f, 1.f, 0.f));

		m_buttons[i].setTextStandardColor(Color(1.f, 1.f, 1.f));
		m_buttons[i].setTextHoveringColor(Color(0.f, 0.f, 0.f));
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
	SoundID id = AudioController::getInstance()->play("bubble_pop", AudioController::SoundType::Effect, true);
	AudioController::getInstance()->fadeOut(id, 10);
}

void MainState::update() {
	Input::getInstance()->setMouseModeAbsolute();

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

	// Logo update
	float offsetX = 1280.f / 16.f;
	float offsetY = 720.f / 6.0f;
	float t = m_timer.getTimePassed();
	for (size_t i = 0; i < m_letters.size(); i++) {
		float2 movement =
			float2(sin(t + m_letters[i].speedOffset.x), cos(t + m_letters[i].speedOffset.y)) * 10.f;
		m_letters[i].letter.setPosition(float2(offsetX, offsetY) + movement);
		offsetX += m_letters[i].letter.getTextureSize().x / (1.65f * 2.f);
	}

	if (m_buttons[btn_start].update_behavior(dt)) {
		//start
		push(State::LevelSelectState);
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
	m_bow.draw();
	for (size_t i = 0; i < m_arrows.size(); i++)
		m_arrows[i]->draw();
	// standard drawing
	sceneManager.draw_color(&m_camera);
	// custom drawing (without dark outline)
	for (size_t i = 0; i < m_arrows.size(); i++)
		m_arrows[i]->draw_trailEffect();

	// Logo
	for (size_t i = 0; i < m_letters.size(); i++)
		m_letters[i].letter.draw();

	// Draw menu buttons
	m_buttons[btn_start].draw();
	m_buttons[btn_settings].draw();
	m_buttons[btn_exit].draw();
	if (DEBUG)
		m_buttons[btn_editor].draw();
}

void MainState::play() {
	sceneManager.load("intro");
	AudioController::getInstance()->play("ocean-music", AudioController::SoundType::Music);
	m_apple = make_shared<Apple>(float3(58.0f, 10.1f, 16.9f));
	m_arrows.clear();
	m_timer.reset();

	// setup buttons
	string buttonTexts[btn_length] = { "Start", "Settings", "Exit", "Editor" };
	float2 btn_pos_start(200, 400);
	float btn_stride_y = 85;
	float btn_delay_stride = 0.1;
	for (size_t i = 0; i < btn_length; i++) {
		m_buttons[i].set(
			btn_pos_start + float2(0, btn_stride_y) * i, buttonTexts[i], btn_delay_stride * i);
	}
}

void MainState::pause() {}

void MainState::restart() {}
