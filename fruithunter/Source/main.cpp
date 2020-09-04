#include <iostream>
#include "ErrorLogger.h"
#include "Quad.h"
#include "Renderer.h"
#include "Input.h"
#include <Windows.h>
#include "VariableSyncer.h"
#include "Settings.h"
#include "PathFindingThread.h"
#include "StateStack.h"

int CALLBACK WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Input::initilize(Renderer::getInstance()->getHandle());

	ErrorLogger errorLogger;
	Input* input = Input::getInstance();
	Renderer* renderer = Renderer::getInstance();
	Settings::initialize();
	PathFindingThread* extraThread = PathFindingThread::getInstance();
	MSG msg = { 0 };

	StateStack stateStack;
	stateStack.push(StateItem::State::MainState);

	// random seed
	srand((unsigned int)time(NULL));

	while (stateStack.isEmpty() == false) {
		VariableSyncer::getInstance()->sync();
		input->update();
		
		// start imgui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Main loop
		stateStack.update();
		renderer->beginFrame();
		stateStack.draw();
		stateStack.handleRequest();

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
					stateStack.pop((StateItem::State)-1, false);//pop all states
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