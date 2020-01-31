#pragma once
#include <wrl/client.h>
#include <Keyboard.h>
#include <Mouse.h>
#include <memory>
#include "Renderer.hpp"
#include "ErrorLogger.hpp"

class Input {
public:
	enum ScrollTracking { DOWN, STILL, UP };
	enum MouseButton { LEFT, MIDDLE, RIGHT };


	~Input();
	static void initilize(HWND window);
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
	bool scrolledUp();
	bool scrolledDown();
	int getMouseMovementX();
	int getMouseMovementY();


	static Input* getInstance();

private:
	Input();
	static Input m_this;
	bool m_isLoaded = false;

	std::unique_ptr<DirectX::Keyboard> m_keyboard;
	std::unique_ptr<DirectX::Mouse> m_mouse;

	DirectX::Keyboard::State m_keyboardState;
	DirectX::Mouse::State m_mouseState;

	DirectX::Keyboard::KeyboardStateTracker m_kbTracker;
	DirectX::Mouse::ButtonStateTracker m_mouseTracker;

	// Scroll wheel tracker
	int m_scrollWheelTracker;
	ScrollTracking m_scrollDirection;

	// Mouse move tracker
	int m_oldX;
	int m_oldY;
};
