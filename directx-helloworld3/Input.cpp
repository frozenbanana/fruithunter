#include "Input.h"


Input::Input(HWND window) {
	m_keyboard = std::make_unique<DirectX::Keyboard>(DirectX::Keyboard()); // Initialize smart pointers
	m_mouse = std::make_unique<DirectX::Mouse>(DirectX::Mouse());
	m_mouse->SetWindow(window);

	update();
}

Input::~Input() {}

void Input::update() {
	m_keyboardState = m_keyboard->GetState();
	m_mouseState = m_mouse->GetState();

	m_kbTracker.Update(m_keyboardState);
	m_mouseTracker.Update(m_mouseState);
}

bool Input::keyPressed(DirectX::Keyboard::Keys key) { return m_kbTracker.IsKeyPressed(key); }
bool Input::keyReleased(DirectX::Keyboard::Keys key) { return m_kbTracker.IsKeyReleased(key); }
bool Input::keyUp(DirectX::Keyboard::Keys key) { return m_keyboardState.IsKeyUp(key); }
bool Input::keyDown(DirectX::Keyboard::Keys key) { return m_keyboardState.IsKeyDown(key); }

bool Input::mousePressed(MouseButton button) {
	switch (button) {
	case LEFT:
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
	case RIGHT:
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
	case MIDDLE:
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
	}
	return false;
}
bool Input::mouseReleased(MouseButton button) {
	switch (button) {
	case LEFT:
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
	case RIGHT:
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
	case MIDDLE:
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
	}
	return false;
}
bool Input::mouseDown(MouseButton button) {
	switch (button) {
	case LEFT:
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::HELD;
	case RIGHT:
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::HELD;
	case MIDDLE:
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::HELD;
	}
	return false;
}
bool Input::mouseUp(MouseButton button) {
	switch (button) {
	case LEFT:
		return m_mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::UP;
	case RIGHT:
		return m_mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::UP;
	case MIDDLE:
		return m_mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::UP;
	}
	return false;
}

int Input::mouseX() { return m_mouseState.x; }

int Input::mouseY() { return m_mouseState.y; }

int Input::scrollWheelValue() { return m_mouseState.scrollWheelValue; }
