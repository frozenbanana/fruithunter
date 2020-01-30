#include "ErrorLogger.hpp"
#include "Quad.hpp"
#include "Renderer.hpp"
#include "Input.hpp"
#include <Windows.h>
#include <stdio.h>
#include "StateHandler.hpp"
#include "PlayState.hpp"
#include "IntroState.hpp"
#include "Camera.hpp"

int CALLBACK WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// Needed to be able to load textures and possibly other things.
	HRESULT ciFlag = CoInitialize(NULL);
	if (FAILED(ciFlag)) {
		ErrorLogger::messageBox("Failed to run CoInitalize(NULL).");
		return -1;
	}

	ErrorLogger errorlogger;
	// Renderer::initalize(window.getHandle());
	Input::initilize(Renderer::getInstance()->getHandle());

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
		stateHandler.update(); // calls current states draw()
		stateHandler.draw();   // calls current states draw()
		Renderer::getInstance()->endFrame();

		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				stateHandler.quit();
			}
		}

		MSG msg = { 0 };
	}

	return 0;
}
