#include "ControlState.h"
#include "Renderer.h"
#include "ErrorLogger.h"

ControlState::ControlState() : StateItem(State::ControlState) {}

ControlState::~ControlState() {}

void ControlState::init() {
	float2 screenSize = float2(1280, 720);

	float2 scale = float2(0.25f);
	float picWidth = 400 * scale.x;
	float pixHeight = 400 * scale.y;
	float padding = 50 * scale.x;
	float2 position = float2(10, 10);
	float animationSpeed = 0.5f;

	// background
	m_background.load("square_white.png");
	m_background.setPosition(float2(1280.f / 2.f, 720.f / 2.f));
	m_background.setSize(screenSize * float2(0.8, 1));
	m_background.setAlignment(); // center
	m_background.setColor(Color(0, 0, 0));
	m_background.setAlpha(0.75);

	// sloMo
	vector<string> strSlowMo = { "hourglassV2_0.png", "hourglassV2_1.png", "hourglassV2_2.png",
		"hourglassV2_1.png" };
	m_slowMo.load(strSlowMo, animationSpeed);
	m_slowMo.setAlignment(); // center
	m_slowMo.set(screenSize / 2 + float2(0, -150), scale * 2.f);
	vector<string> strSlowMoKey = { "btnF.png", "btnF_pressed.png" };
	m_slowMoKey.load(strSlowMoKey);
	m_slowMoKey.setAlignment(); // center
	m_slowMoKey.set(screenSize / 2 + float2(0,175) +float2(0,-150), float2(1, 1) * 0.35);

	// shooting
	vector<string> strMouseLeft = { "mouse.png", "mouse_left.png" };
	m_mouseLeft.load(strMouseLeft, animationSpeed);
	m_mouseLeft.setAlignment();
	m_mouseLeft.set(float2(1280 - (1070 - 100), 50) + float2(0,350), float2(1.f) * 0.4f);
	vector<string> strBow = { "bow_unstreched.png", "bow_streched.png" };
	m_bow.load(strBow, animationSpeed);
	m_bow.setAlignment();
	m_bow.set(float2(1280 - (1070 - 100), 50)+float2(0,150), float2(1.f) * 0.6f);

	// movement keys
	vector<string> strMovementTex = {
		"CharWalk0.png",
		"CharWalk1.png",
		"CharWalk2.png",
		"CharWalk1.png",
	};
	m_charMovement.load(strMovementTex, animationSpeed);
	m_charMovement.set(float2(1070-100, 140+50), float2(0.5f));
	m_charMovement.setAlignment(); // center
	strMovementTex = { "movement.png", "movement_w.png", "movement_a.png", "movement_s.png",
		"movement_d.png" };
	m_movementKeys.load(strMovementTex, animationSpeed);
	m_movementKeys.set(float2(1070-100, 310+50), float2(0.5f));
	m_movementKeys.setAlignment(); // center

	// jump key
	vector<string> strJumpTex = { "charJump0.png", "charJump1.png" };
	m_charJump.load(strJumpTex, animationSpeed);
	m_charJump.set(float2(1070-100, 500), float2(0.5));
	m_charJump.setAlignment(); // Center
	strJumpTex = { "jump_up.png", "jump_down.png" };
	m_jumpKey.load(strJumpTex, animationSpeed);
	m_jumpKey.set(float2(1070-100, 620), float2(0.5));
	m_jumpKey.setAlignment(); // Center
}

void ControlState::update(double dt) {
	Input::getInstance()->setMouseModeAbsolute();

	if (m_btn_back.update_behavior(dt)) {
		pop(false);
	}
}

void ControlState::draw() {
	Renderer::getInstance()->drawCapturedFrame();
	m_background.draw();
	
	// shooting
	m_text.setScale(0.6f);
	m_text.setAlignment();
	m_text.setPosition(float2(1280-(1070-100), 50));
	m_text.setText("Shooting");
	m_text.draw();
	m_mouseLeft.draw();
	m_bow.draw();

	// slowmotion
	m_text.setScale(0.6f);
	m_text.setAlignment();
	m_text.setPosition(float2(1280 / 2, 50));
	m_text.setText("Slow Motion");
	m_text.draw();
	m_slowMo.draw();
	m_slowMoKey.draw();
	m_text.setScale(0.25f);
	m_text.setAlignment();
	m_text.setPosition(float2(1280 / 2, 450));
	m_text.setText("(Press  twice  to  turn  off)");
	m_text.draw();

	// movement
	m_text.setScale(0.6f);
	m_text.setAlignment();
	m_text.setPosition(float2(1070 - 100, 50));
	m_text.setText("Movement");
	m_text.draw();
	m_movementKeys.draw();
	m_charMovement.draw();
	m_jumpKey.draw();
	m_charJump.draw();
	m_text.setScale(0.25f);
	m_text.setAlignment();
	m_text.setPosition(float2(1070 - 100, 720 - 50));
	m_text.setText("(Press  twice  to  double  jump)");
	m_text.draw();

	// buttons
	m_btn_back.draw();
}

void ControlState::pause() {}

void ControlState::play() {
	m_btn_back.set(float2(1280/2, 720-50), "Back", 0.2);
}

void ControlState::restart() {}
