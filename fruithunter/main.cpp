#include "ErrorLogger.hpp"
#include "Quad.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "Input.hpp"
#include <Windows.h>
#include <stdio.h>
#include "StateHandler.hpp"
#include "PlayState.hpp"
#include "IntroState.hpp"
#include "Camera.hpp"

int CALLBACK WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount) {

	// Needed to be able to load textures and possibly other things.
	HRESULT ciFlag = CoInitialize(NULL);
	if (FAILED(ciFlag)) {
		ErrorLogger::messageBox("Failed to run CoInitalize(NULL).");
		return -1;
	}

	ErrorLogger errorlogger;
	Window window(800, 600);
	Renderer::initalize(window.getHandle());
	Input::initilize(window.getHandle());


	Camera myCam = Camera();

	// Example of how to do logging
	ErrorLogger::log("First");
	ErrorLogger::logWarning(ciFlag, "Second!");
	ErrorLogger::logError(ciFlag, "Third!");

	StateHandler stateHandler;
	stateHandler.pushState(PlayState::getInstance());
	stateHandler.pushState(IntroState::getInstance());

	MSG msg = { 0 };
	while (stateHandler.isRunning()) {
		Input::getInstance()->update();
		if (Input::getInstance()->keyPressed(DirectX::Keyboard::D1)) {
			ErrorLogger::log("Number 1 was pressed!");
			stateHandler.changeState(IntroState::getInstance());
		}

		if (Input::getInstance()->keyPressed(DirectX::Keyboard::D2)) {
			ErrorLogger::log("Number 2 was pressed!");
			stateHandler.changeState(PlayState::getInstance());
		}

		// Main loop
		Renderer::getInstance()->beginFrame();
		stateHandler.draw(); // calls current states draw()
		Renderer::getInstance()->endFrame();

		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				stateHandler.quit();
			}
		}
	}

	return 0;
}
