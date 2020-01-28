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

// void Input::processMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
//	switch (message) {
//	case WM_ACTIVATEAPP:
//		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
//		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
//		break;
//
//	case WM_INPUT:
//	case WM_MOUSEMOVE:
//	case WM_LBUTTONDOWN:
//	case WM_LBUTTONUP:
//	case WM_RBUTTONDOWN:
//	case WM_RBUTTONUP:
//	case WM_MBUTTONDOWN:
//	case WM_MBUTTONUP:
//	case WM_MOUSEWHEEL:
//	case WM_XBUTTONDOWN:
//	case WM_XBUTTONUP:
//	case WM_MOUSEHOVER:
//		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
//		break;
//
//	case WM_KEYDOWN:
//	case WM_SYSKEYDOWN:
//	case WM_KEYUP:
//	case WM_SYSKEYUP:
//		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
//		break;
//	}
//}

bool Input::isPressed(DirectX::Keyboard::Keys key) { return m_kbTracker.IsKeyPressed(key); }
bool Input::isReleased(DirectX::Keyboard::Keys key) { return m_kbTracker.IsKeyReleased(key); }
bool Input::isUp(DirectX::Keyboard::Keys key) { return m_keyboardState.IsKeyUp(key); }
bool Input::isDown(DirectX::Keyboard::Keys key) { return m_keyboardState.IsKeyDown(key); }
