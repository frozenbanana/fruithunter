#include "ErrorLogger.h"
#include "Quad.h"
#include "Renderer.h"
#include "Input.h"
#include <Windows.h>
#include <stdio.h>
#include "StateHandler.h"
#include "PlayState.h"
#include "IntroState.h"
#include "Camera.h"

int CALLBACK WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Input::initilize(Renderer::getInstance()->getHandle());

	StateHandler* stateHandler = StateHandler::getInstance();
	Input* input = Input::getInstance();
	Renderer* renderer = Renderer::getInstance();

	MSG msg = { 0 };
	stateHandler->initialize();
	while (StateHandler::getInstance()->isRunning()) {
		input->update();
		if (input->keyPressed(DirectX::Keyboard::D1)) {
			ErrorLogger::log("Number 1 was pressed!");
			stateHandler->changeState(StateHandler::INTRO);
		}

		if (input->keyPressed(DirectX::Keyboard::D2)) {
			ErrorLogger::log("Number 2 was pressed!");
			stateHandler->changeState(StateHandler::PLAY);
		}

		// Main loop
		renderer->beginFrame();
		stateHandler->handleEvent(); // calls current states draw()
		stateHandler->update();		 // calls current states draw()
		stateHandler->draw();		 // calls current states draw()
		renderer->endFrame();

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				stateHandler->quit();
				break;
			}
		}

		if (Input::getInstance()->keyPressed(DirectX::Keyboard::Escape)) {
			stateHandler->quit();
			break;
		}
		MSG msg = { 0 };
	}

	return 0;
}
