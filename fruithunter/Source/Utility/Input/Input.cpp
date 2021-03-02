#include "Input.h"
#include "ErrorLogger.h"
#include "Renderer.h"


Input Input::m_this;

Input::Input() {}

Input::~Input() {}

void Input::initilize(HWND window) {
	Input* inp = Input::getInstance();
	if (!inp->m_isLoaded) {
		inp->m_keyboard =
			std::make_unique<DirectX::Keyboard>(DirectX::Keyboard()); // Initialize smart pointers
		inp->m_mouse = std::make_unique<DirectX::Mouse>(DirectX::Mouse());
		inp->m_mouse->SetWindow(window);
		inp->m_scrollWheelTracker = 0;
		inp->update();
		inp->m_isLoaded = true;
	}
}

// Updates states and trackers
void Input::update() {
	// Update current state
	m_keyboardState = m_keyboard->GetState();
	m_mouseState = m_mouse->GetState();

	// Track change in scrollwheel
	if (m_scrollWheelTracker == m_mouseState.scrollWheelValue)
		m_scrollDirection = ScrollTracking::STILL;
	else if (m_scrollWheelTracker < m_mouseState.scrollWheelValue)
		m_scrollDirection = ScrollTracking::DOWN;
	else
		m_scrollDirection = ScrollTracking::UP;

	m_kbTracker.Update(m_keyboardState);
	m_mouseTracker.Update(m_mouseState);
	m_scrollWheelTracker = m_mouseState.scrollWheelValue;
}

bool Input::keyPressed(DirectX::Keyboard::Keys key, bool filterImgui) {
	if (m_imguiInput && filterImgui && ImGui::GetIO().WantCaptureKeyboard)
		return false;
	return m_kbTracker.IsKeyPressed(key);
}
bool Input::keyReleased(DirectX::Keyboard::Keys key, bool filterImgui) {
	if (m_imguiInput && filterImgui && ImGui::GetIO().WantCaptureKeyboard)
		return false;
	return m_kbTracker.IsKeyReleased(key);
}
bool Input::keyUp(DirectX::Keyboard::Keys key, bool filterImgui) {
	if (m_imguiInput && filterImgui && ImGui::GetIO().WantCaptureKeyboard)
		return false;
	return m_keyboardState.IsKeyUp(key);
}
bool Input::keyDown(DirectX::Keyboard::Keys key, bool filterImgui) {
	if (m_imguiInput && filterImgui && ImGui::GetIO().WantCaptureKeyboard)
		return false;
	return m_keyboardState.IsKeyDown(key);
}

bool Input::mousePressed(MouseButton button, bool filterImgui) {
	if (m_imguiInput && filterImgui && ImGui::GetIO().WantCaptureMouse)
		return false;
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
bool Input::mouseReleased(MouseButton button, bool filterImgui) {
	if (m_imguiInput && filterImgui && ImGui::GetIO().WantCaptureMouse)
		return false;
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
bool Input::mouseDown(MouseButton button, bool filterImgui) {
	if (m_imguiInput && filterImgui && ImGui::GetIO().WantCaptureMouse)
		return false;
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
bool Input::mouseUp(MouseButton button, bool filterImgui) {
	if (m_imguiInput && filterImgui && ImGui::GetIO().WantCaptureMouse)
		return false;
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

float Input::mouseX() const { 
	Renderer* r = Renderer::getInstance();
	if (r->isFullscreen() && m_mouseState.positionMode == Mouse::Mode::MODE_ABSOLUTE) {
		// remap mouse position to be in screen position 
		// (button collisions require mouse position to be within screen size)
		return ((float)m_mouseState.x / r->getWindowWidth()) * r->getScreenWidth();
	}
	return (float)m_mouseState.x; 
}

float Input::mouseY() const { 
	Renderer* r = Renderer::getInstance();
	if (r->isFullscreen() && m_mouseState.positionMode == Mouse::Mode::MODE_ABSOLUTE) {
		// remap mouse position to be in screen position 
		// (button collisions require mouse position to be within screen size)
		return ((float)m_mouseState.y / r->getWindowHeight()) * r->getScreenHeight();
	}
	return (float)m_mouseState.y; 
}

float2 Input::mouseXY() const { return float2(mouseX(), mouseY()); }

int Input::scrollWheelValue() { return m_mouseState.scrollWheelValue; }

bool Input::scrolledUp() { return m_scrollDirection == ScrollTracking::DOWN; }

bool Input::scrolledDown() { return m_scrollDirection == ScrollTracking::UP; }

XMINT2 Input::getMouseMovement() const { return m_mouseMovement; }

DirectX::Mouse::Mode Input::getMouseMode() { return m_mouseState.positionMode; }

void Input::enableImguiInput() { m_imguiInput = true; }

void Input::disableImguiInput() { m_imguiInput = false; }

bool Input::isImguiReceivingInput() { return m_imguiInput; }

void Input::event_mouseInput(RAWMOUSE mouse_event) {
	m_mouseMovement =
		XMINT2(m_mouseMovement.x + mouse_event.lLastX, m_mouseMovement.y + mouse_event.lLastY);
}

void Input::event_frameReset() { m_mouseMovement = XMINT2(); }

Input* Input::getInstance() { return &m_this; }


void Input::setMouseModeRelative() {
	disableImguiInput();
	if (m_mouseState.positionMode == DirectX::Mouse::MODE_ABSOLUTE)
		m_mouse.get()->SetMode(DirectX::Mouse::MODE_RELATIVE);

	if (m_mouse.get()->IsVisible() == true)
		m_mouse.get()->SetVisible(false);
}

void Input::setMouseModeAbsolute() {
	enableImguiInput();
	if (m_mouseState.positionMode != DirectX::Mouse::MODE_ABSOLUTE)
		m_mouse.get()->SetMode(DirectX::Mouse::MODE_ABSOLUTE);

	if (m_mouse.get()->IsVisible() == false)
		m_mouse.get()->SetVisible(true);
}
