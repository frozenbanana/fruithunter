#pragma once
#include "GlobalNamespaces.h"
#include <Keyboard.h>
#include <Mouse.h>
#include <memory>

class Input {
public:
	enum ScrollTracking { DOWN, STILL, UP };
	enum MouseButton { LEFT, MIDDLE, RIGHT };
	
	~Input();
	static void initilize(HWND window);
	void update();
	// Input for keyboard
	bool keyPressed(DirectX::Keyboard::Keys key, bool filterImgui = true);
	bool keyReleased(DirectX::Keyboard::Keys key, bool filterImgui = true);
	bool keyDown(DirectX::Keyboard::Keys key, bool filterImgui = true);
	bool keyUp(DirectX::Keyboard::Keys key, bool filterImgui = true);

	// Input for mouse
	bool mousePressed(MouseButton button, bool filterImgui = true);
	bool mouseReleased(MouseButton button, bool filterImgui = true);
	bool mouseDown(MouseButton button, bool filterImgui = true);
	bool mouseUp(MouseButton button, bool filterImgui = true);
	int mouseX();
	int mouseY();
	int scrollWheelValue();
	bool scrolledUp();
	bool scrolledDown();
	int getMouseMovementX();
	int getMouseMovementY();

	static Input* getInstance();

	void setMouseModeRelative();
	void setMouseModeAbsolute();
	DirectX::Mouse::Mode getMouseMode();

	void enableImguiInput();
	void disableImguiInput();
	bool isImguiReceivingInput();

private:
	Input();
	static Input m_this;
	bool m_isLoaded = false;
	bool m_imguiInput = false;

	std::unique_ptr<DirectX::Keyboard> m_keyboard = nullptr;
	std::unique_ptr<DirectX::Mouse> m_mouse = nullptr;

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
