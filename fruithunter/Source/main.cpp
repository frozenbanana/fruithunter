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
#include "VariableSyncer.h"
#include "PerformanceTimer.h"

void onLoad(void* ptr) { ErrorLogger::log("Loaded struct!"); }

int CALLBACK WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	PerformanceTimer::start("Initilize");

	Input::initilize(Renderer::getInstance()->getHandle());

	StateHandler* stateHandler = StateHandler::getInstance();
	Input* input = Input::getInstance();
	Renderer* renderer = Renderer::getInstance();
	ErrorLogger errorLogger;

	MSG msg = { 0 };
	stateHandler->initialize();

	PerformanceTimer::stop();
	PerformanceTimer::start("AllFrames");

	// Hardcoded statechange here. (TESTING)
	// stateHandler->changeState(StateHandler::PLAY);
	while (StateHandler::getInstance()->isRunning()) {
		PerformanceTimer::start("FrameTime", PerformanceTimer::TimeState::state_average);
		VariableSyncer::getInstance()->sync();
		input->update();
		if (input->keyPressed(DirectX::Keyboard::F1)) {
			ErrorLogger::log("Number 1 was pressed!");
			stateHandler->changeState(StateHandler::INTRO);
		}

		if (input->keyPressed(DirectX::Keyboard::F2)) {
			ErrorLogger::log("Number 2 was pressed!");
			stateHandler->changeState(StateHandler::PLAY);
		}

		if (input->keyPressed(DirectX::Keyboard::Escape)) {
			ErrorLogger::log("Number 3 was pressed!");
			stateHandler->changeState(StateHandler::PAUSE);
		}

		// Main loop
		stateHandler->handleEvent();
		stateHandler->update();
		stateHandler->draw(); // calls current states draw()
		renderer->endFrame();

		PerformanceTimer::stop();
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				stateHandler->quit();
				break;
			}
		}

		if (Input::getInstance()->keyPressed(DirectX::Keyboard::F4)) {
			stateHandler->quit();
			break;
		}
		MSG msg = { 0 };
	}
	PerformanceTimer::stop();
	return 0;
}
