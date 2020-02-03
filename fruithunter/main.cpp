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

#include <Audio.h>

int CALLBACK WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// Needed to be able to load textures and possibly other things.
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		ErrorLogger::messageBox("Failed to run CoInitalize(NULL).");
		return -1;
	}

	ErrorLogger errorlogger;
	// Renderer::initalize(window.getHandle());
	Input::initilize(Renderer::getInstance()->getHandle());

	StateHandler* stateHandler = StateHandler::getInstance();
	stateHandler->initialize();

	MSG msg = { 0 };
	while (StateHandler::getInstance()->isRunning()) {
		Input::getInstance()->update();
		if (Input::getInstance()->keyPressed(DirectX::Keyboard::D1)) {
			ErrorLogger::log("Number 1 was pressed!");
			stateHandler->changeState(StateHandler::INTRO);
		}

		if (Input::getInstance()->keyPressed(DirectX::Keyboard::D2)) {
			ErrorLogger::log("Number 2 was pressed!");
			stateHandler->changeState(StateHandler::PLAY);
		}

		// Main loop
		Renderer::getInstance()->beginFrame();
		stateHandler->update(); // calls current states draw()
		stateHandler->draw();	// calls current states draw()
		Renderer::getInstance()->endFrame();

		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				stateHandler->quit();
			}
		}

		MSG msg = { 0 };
	}

	return 0;
}
