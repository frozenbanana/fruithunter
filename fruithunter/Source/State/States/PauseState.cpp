#include "PauseState.h"
#include "Renderer.h"
#include "Input.h"
#include "AudioController.h"

PauseState::PauseState() : StateItem(State::PauseState) { }

PauseState::~PauseState() {}

void PauseState::init() {
	m_settingsBackground.load("apple.png");
	m_settingsBackground.setPosition(float2(1280.f / 2.f, 720.f / 2.f));
	m_settingsBackground.setScale(1.25f);
	m_settingsBackground.setAlignment();//center

	float2 scale = float2(0.25f);
	float picWidth = 400 * scale.x;
	float pixHeight = 400 * scale.y;
	float padding = 50 * scale.x;
	float2 position = float2(10, 10);
	float animationSpeed = 0.5f;
	//pick fruits
	vector<vector<string>> strTextures = { 
		{ "drop_hold.png", "drop_releaseApple.png" },
		{ "drop_hold.png", "drop_releaseBanana.png" },
		{ "drop_hold.png", "drop_releaseMelon.png" },
		{ "drop_hold.png", "drop_releaseDragonFruit.png" }
	};
	for (size_t i = 0; i < 4; i++) {
		m_dropFruits[i].load(strTextures[i], animationSpeed);
		m_dropFruits[i].set(position, scale);
		position.x += picWidth + padding;
	}

	//sloMo
	vector<string> strSlowMo = { "hourglassV2_0.png", "hourglassV2_1.png", "hourglassV2_2.png",
		"hourglassV2_1.png" };
	m_slowMo.load(strSlowMo, animationSpeed);
	m_slowMo.set(position - m_slowMo.getSize() * scale * 0.5f * 0.3f - float2(0, -10.f), scale * 1.3f);
	
	//Buttons
	position = float2(10, 10);
	position.y += pixHeight + padding;
	vector<vector<string>> strTextureButtons = { { "btn1.png", "btn1_pressed.png" },
		{ "btn2.png", "btn2_pressed.png" }, { "btn3.png", "btn3_pressed.png" },
		{ "btn4.png", "btn4_pressed.png" }, { "btnF.png", "btnF_pressed.png" } };
	for (size_t i = 0; i < strTextureButtons.size(); i++) {
		m_btns[i].load(strTextureButtons[i], animationSpeed);
		m_btns[i].set(position, scale);
		position.x += picWidth + padding;
	}
	for (size_t i = 0; i < btn_length; i++) {
		m_buttons[i].setStandardColor(Color(42.f / 255.f, 165.f / 255.f, 209.f / 255.f));
		m_buttons[i].setHoveringColor(Color(1.f, 210.f / 255.f, 0.f));

		m_buttons[i].setTextStandardColor(Color(1.f, 1.f, 1.f));
		m_buttons[i].setTextHoveringColor(Color(0.f, 0.f, 0.f));
	}

	//shooting
	vector<string> strMouseLeft = { "mouse.png", "mouse_left.png" };
	m_mouseLeft.load(strMouseLeft, animationSpeed);
	m_mouseLeft.set(float2(10, 270), float2(1.f) * 0.4f);
	vector<string> strBow = { "bow_unstreched.png", "bow_streched.png" };
	m_bow.load(strBow, animationSpeed);
	m_bow.set(float2(140, 230), float2(1.f) * 0.6f);

	// movement keys
	vector<string> strMovementTex = {
		"CharWalk0.png",
		"CharWalk1.png",
		"CharWalk2.png",
		"CharWalk1.png",
	};
	m_charMovement.load(strMovementTex, animationSpeed);
	m_charMovement.set(float2(1070, 140), float2(0.5f));
	m_charMovement.setAlignment();//center
	strMovementTex = { "movement.png", "movement_w.png", "movement_a.png", "movement_s.png", "movement_d.png" };
	m_movementKeys.load(strMovementTex, animationSpeed);
	m_movementKeys.set(float2(1070, 310), float2(0.5f));
	m_movementKeys.setAlignment();//center

	//jump key
	vector<string> strJumpTex = { "charJump0.png", "charJump1.png" };
	m_charJump.load(strJumpTex, animationSpeed);
	m_charJump.set(float2(1070, 500), float2(0.5));
	m_charJump.setAlignment();//Center
	strJumpTex = { "jump_up.png", "jump_down.png" };
	m_jumpKey.load(strJumpTex, animationSpeed);
	m_jumpKey.set(float2(1070, 620), float2(0.5));
	m_jumpKey.setAlignment();//Center
}

void PauseState::update() {
	Input::getInstance()->setMouseModeAbsolute();

	m_timer.update();
	float dt = m_timer.getDt();

	if (m_buttons[btn_restart].update_behavior(dt)) {
		pop(true);
	}
	if (m_buttons[btn_resume].update_behavior(dt) || Input::getInstance()->keyPressed(Keyboard::Escape)) {
		pop(false);
	}
	if (m_buttons[btn_settings].update_behavior(dt)) {
		push(State::SettingState);
	}
	if (m_buttons[btn_mainmenu].update_behavior(dt)) {
		AudioController::getInstance()->flush();
		pop(State::MainState, false);
	}
	if (m_buttons[btn_exit].update_behavior(dt)) {
		AudioController::getInstance()->flush();
		pop((State)-1, false); // will pop all states, resulting in an empty stack}
	}
}

void PauseState::pause() { }

void PauseState::play() {
	Renderer::getInstance()->captureFrame();

	//m_settingsBackground.setPosition(float2(width / 2.f, height / 2.f));

	// setup buttons
	string buttonTexts[btn_length] = { "Restart", "Resume", "Settings", "Main Menu", "Exit" };
	float2 btn_pos_start(1280/2.f, 275);
	float btn_stride_y = 80;
	float btn_delay_stride = 0.1;
	for (size_t i = 0; i < btn_length; i++) {
		m_buttons[i].set(
			btn_pos_start + float2(0, btn_stride_y) * i, buttonTexts[i], btn_delay_stride * (i+1));
		m_buttons[i].setDesiredScale_hovering(0.8);
		m_buttons[i].setDesiredScale_standard(0.7);
	}
}

void PauseState::restart() {}


void PauseState::draw() {
	Renderer::getInstance()->beginFrame();
	Renderer::getInstance()->drawCapturedFrame();
	m_settingsBackground.drawNoScaling();

	for (size_t i = 0; i < 4; i++) {
		m_dropFruits[i].draw();
		m_btns[i].draw();
	}
	m_slowMo.draw();
	m_btns[4].draw();
	m_movementKeys.draw();
	m_charMovement.draw();
	m_jumpKey.draw();
	m_charJump.draw();
	m_mouseLeft.draw();
	m_bow.draw();

	for (size_t i = 0; i < btn_length; i++)
		m_buttons[i].draw();
}
