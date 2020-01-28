#include "ErrorLogger.hpp"
#include "Quad.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "Input.h"
#include <Windows.h>
#include <stdio.h>
#include "StateHandler.hpp"
#include "PlayState.hpp"
#include "IntroState.hpp"

int CALLBACK WinMain(
	_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine, _In_ int cmdCount) {

	// Needed to be able to load textures and possibly other things.
	HRESULT ciFlag = CoInitialize(NULL);
	if (FAILED(ciFlag)) {
		ErrorLogger::messageBox("Failed to run CoInitalize(NULL).");
		return -1;
	}

	ErrorLogger errorlogger;
	Window window(800, 600);
	Renderer renderer(window);
	Quad quad(renderer);
	Input input(window.getHandle());


	ErrorLogger::log("First");
	ErrorLogger::logWarning(ciFlag, "Second!");
	ErrorLogger::logError(ciFlag, "Third!");

	StateHandler stateHandler;
	stateHandler.pushState(IntroState::getInstance());
	stateHandler.pushState(PlayState::getInstance());


	MSG msg = { 0 };
	while (stateHandler.isRunning()) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				stateHandler.quit();
			}

			if (msg.message == WM_) {
				stateHandler.quit();
			}

			// Main loop
			stateHandler.update();
			stateHandler.draw();

			renderer.beginFrame();
			quad.draw(renderer);
			renderer.endFrame();
		}
	}


	MSG msg = { 0 };
	while (true) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				break;
			}
		}

		input.update();

		// Main loop
		renderer.beginFrame();
		quad.draw(renderer);
		renderer.endFrame();
	}

	return 0;
}
