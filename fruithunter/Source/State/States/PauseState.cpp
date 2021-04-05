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
	if (m_buttons[btn_controls].update_behavior(dt)) {
		push(State::ControlState);
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
	// setup buttons
	string buttonTexts[btn_length] = { "Restart", "Resume", "Controls", "Settings", "Main Menu", "Exit" };
	float2 btn_pos_start(1280/2.f, 275-50);
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
	Renderer::getInstance()->drawCapturedFrame();
	m_settingsBackground.draw();

	for (size_t i = 0; i < btn_length; i++)
		m_buttons[i].draw();
}
