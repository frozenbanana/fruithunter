#pragma once
#include <wrl/client.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <memory>
#include "Window.hpp"
class Input {
private:
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;

	DirectX::Keyboard::State m_keyboardState;
	DirectX::Mouse::State m_mouseState;

	DirectX::Keyboard::KeyboardStateTracker m_kbTracker;
	DirectX::Mouse::ButtonStateTracker m_mouseTracker;

public:
	Input(HWND window);
	~Input();
	void update();
	// void processMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	// Input for keyboard
	bool isPressed(DirectX::Keyboard::Keys key);
	bool isReleased(DirectX::Keyboard::Keys key);
	bool isDown(DirectX::Keyboard::Keys key);
	bool isUp(DirectX::Keyboard::Keys key);

	// bool isPressed(DirectX::Keyboard::Keys key);
	// bool isReleased(DirectX::Keyboard::Keys key);
	// bool isDown(DirectX::Keyboard::Keys key);
	// bool isUp(DirectX::Keyboard::Keys key);
};
