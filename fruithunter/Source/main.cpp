#include "ErrorLogger.h"
#include "Quad.h"
#include "Renderer.h"
#include "Input.h"
#include <Windows.h>
#include <stdio.h>
#include "StateHandler.h"
#include "VariableSyncer.h"
#include "Settings.h"
#include "PathFindingThread.h"

int CALLBACK WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Input::initilize(Renderer::getInstance()->getHandle());

	ErrorLogger errorLogger;
	StateHandler* stateHandler = StateHandler::getInstance();
	Input* input = Input::getInstance();
	Renderer* renderer = Renderer::getInstance();
	PathFindingThread* extraThread = PathFindingThread::getInstance();
	MSG msg = { 0 };
	stateHandler->initialize();

	// random seed
	srand((unsigned int)time(NULL));

	while (StateHandler::getInstance()->isRunning()) {
		VariableSyncer::getInstance()->sync();
		input->update();
		
		// start imgui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Main loop
		stateHandler->handleEvent();
		stateHandler->update();
		renderer->beginFrame();
		stateHandler->draw();

		// imgui assemble draw data
		ImGui::Render();
		// imgui Render Draw data
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		renderer->endFrame();

		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			switch (msg.message) {
				case WM_QUIT: {
					stateHandler->quit();
					break;
				}
			}
		}

		MSG msg = { 0 };
	}

	Settings::getInstance()->saveAllSetting();
	VariableSyncer::getInstance()->saveAll();
	return 0;
}