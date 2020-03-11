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
#include "PathFindingThread.h"
#include "Settings.h"

int CALLBACK WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Input::initilize(Renderer::getInstance()->getHandle());
	Settings::initialize();

	StateHandler* stateHandler = StateHandler::getInstance();
	Input* input = Input::getInstance();
	Renderer* renderer = Renderer::getInstance();
	ErrorLogger errorLogger;

	PathFindingThread* extraThread = PathFindingThread::getInstance();

	MSG msg = { 0 };
	stateHandler->initialize();

	while (StateHandler::getInstance()->isRunning()) {
		VariableSyncer::getInstance()->sync();
		input->update();

		// Main loop
		stateHandler->handleEvent();
		stateHandler->update();
		stateHandler->draw();
		renderer->endFrame();

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				stateHandler->quit();
				break;
			}
		}

		MSG msg = { 0 };
	}
	extraThread->exitThread();
	return 0;
}
