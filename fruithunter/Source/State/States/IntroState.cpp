#include "IntroState.h"
#include "ErrorLogger.h"
#include "AudioHandler.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"
#include "Settings.h"

IntroState::IntroState() { initialize(); }

IntroState::~IntroState() {}

void IntroState::initialize() {
	m_name = "Intro State";
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;

	m_startButton.initialize("Start", float2(106, height * 0.75f - 60.f));
	m_settingsButton.initialize("Settings", float2(132, height * 0.75f));
	m_exitButton.initialize("Exit", float2(92, height * 0.75f + 60.f));

	m_bow.setRecoveryTime(0);

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
		m_letters[i].letter.setAlignment();//center
	}

	m_timer.reset();
}

void IntroState::update() {
	float3 treePos(56.4f, 9.0f, 18.2f);
	float3 bowPos = treePos + float3(10,1.5,5);

	m_timer.update();
	float delta = m_timer.getDt();
	m_totalDelta = fmod((m_totalDelta + delta), (2.f * XM_PI));
	m_totalDelta_forBow += delta;

	Input::getInstance()->setMouseModeAbsolute();

	// update scene
	sceneManager.update(&m_camera);

	// update precoded bow behavior
	float3 target = treePos + float3(0, 2.0, 0) +
					float3(RandomFloat(-1, 1), RandomFloat(-1, 1), RandomFloat(-1, 1));
	float3 bowForward = target - bowPos;
	bowForward.Normalize();
	float3 rot = vector2Rotation(bowForward);
	m_bow.update_rotation(rot.x, rot.y);
	m_bow.update_positioning(delta, bowPos, bowForward, bowForward.Cross(float3(0.f, 1.0f, 0.f)));
	shared_ptr<Arrow> arrow;
	if (m_totalDelta_forBow >= m_shootDelay + m_bowHoldTime) {
		m_totalDelta_forBow = 0; // reset timer
		//randomize bow values
		//m_bowHoldTime = RandomFloat(1.0, 1.5);
	}
	else if (m_totalDelta_forBow >= m_bowHoldTime) {
		arrow = m_bow.update_bow(delta, false); // release string
	}
	else {
		arrow = m_bow.update_bow(delta, true);// pull string
	}
	if (arrow.get() != nullptr)
		m_arrows.push_back(arrow);// add shot arrow to array

	// arrow collision
	vector<unique_ptr<Entity>>* entities = SceneManager::getScene()->m_repository.getEntities();
	for (size_t i = 0; i < m_arrows.size(); i++) {
		if (m_arrows[i]->isActive()) {
			// !! if arrow collides with anything, then the arrow handles the behavior !!
			// check collision with terrains and static entities
			m_arrows[i]->collide_terrainBatch(delta, sceneManager.getScene()->m_terrains);
			for (size_t j = 0; j < entities->size(); j++) {
				m_arrows[i]->collide_entity(delta, *(*entities)[j]);
			}
		}
		// update arrow
		m_arrows[i]->update(delta);
	}

	// update apple behavior (run around tree)
	Fruit* fruit = m_apple.get();
	fruit->setPosition(float3(
		treePos.x + (cos(m_totalDelta) * 2.0f), treePos.y, treePos.z + (sin(m_totalDelta) * 2.0f)));
	fruit->update(delta, m_camera.getPosition());
	fruit->setRotation(float3(0.0f, -m_totalDelta, 0.0f));

	// Logo update
	float offsetX = 1280.f / 16.f;
	float offsetY = 720.f / 3.f;
	float t = m_timer.getTimePassed();
	for (size_t i = 0; i < m_letters.size(); i++) {
		float2 movement =
			float2(sin(t + m_letters[i].speedOffset.x), cos(t + m_letters[i].speedOffset.y)) * 10.f;
		m_letters[i].letter.setPosition(float2(offsetX, offsetY) + movement);
		offsetX += m_letters[i].letter.getTextureSize().x / (1.65f * 2.f);
	}

	Input::getInstance()->setMouseModeAbsolute();
}

void IntroState::handleEvent() {
	if (m_startButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::LEVEL_SELECT);
	}
	if (m_settingsButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::SETTINGS);
	}
	if (m_exitButton.update()) {
		StateHandler::getInstance()->quit();
	}
}

void IntroState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::getInstance()->pauseAllMusic();
}

void IntroState::draw() {

	//	__SHADOWS__
	sceneManager.setup_shadow(&m_camera);
	//custom shadow drawing
	m_apple->draw_animate_onlyMesh();
	for (size_t i = 0; i < m_arrows.size(); i++)
		m_arrows[i]->draw_onlyMesh(float3(1.));
	//standard shadow drawing
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
	sceneManager.draw_color();
	// custom drawing (without dark outline)
	for (size_t i = 0; i < m_arrows.size(); i++)
		m_arrows[i]->draw_trailEffect();

	// Logo
	for (size_t i = 0; i < m_letters.size(); i++)
		m_letters[i].letter.draw();

	// Draw menu buttons
	m_startButton.draw();
	m_settingsButton.draw();
	m_exitButton.draw();
}


void IntroState::play() {
	ErrorLogger::log(m_name + " play() called.");

	Settings::getInstance()->loadAllSetting();

	AudioHandler::getInstance()->playMusic(AudioHandler::Music::OCEAN);

	sceneManager.load("intro");
	m_apple = make_shared<Apple>(float3(58.0f, 10.1f, 16.9f));
	m_arrows.clear();
	m_camera.setView(
		float3(58.0f, 10.9f, 21.9f), float3(61.3f, 10.1f, -36.0f), float3(0.f, 1.f, 0.f));
}
