#pragma once
#include <wrl/client.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <memory>
#include "Window.hpp"
#include "ErrorLogger.hpp"

class Input {
private:
	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;

	DirectX::Keyboard::State m_keyboardState;
	DirectX::Mouse::State m_mouseState;

	DirectX::Keyboard::KeyboardStateTracker m_kbTracker;
	DirectX::Mouse::ButtonStateTracker m_mouseTracker;

public:
	enum MouseButton { LEFT, MIDDLE, RIGHT };

	Input(HWND window);
	~Input();
	void update();

	// Input for keyboard
	bool keyPressed(DirectX::Keyboard::Keys key);
	bool keyReleased(DirectX::Keyboard::Keys key);
	bool keyDown(DirectX::Keyboard::Keys key);
	bool keyUp(DirectX::Keyboard::Keys key);

	// Input for mouse
	bool mousePressed(MouseButton button);
	bool mouseReleased(MouseButton button);
	bool mouseDown(MouseButton button);
	bool mouseUp(MouseButton button);
	int mouseX();
	int mouseY();
	int scrollWheelValue();
};
