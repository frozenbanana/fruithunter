#include <iostream>
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Input.h"
#include <Windows.h>
#include "VariableSyncer.h"
#include "Settings.h"
#include "PathFindingThread.h"
#include "StateStack.h"
#include "AudioController.h"
#include "SimpleDirectX.h"

#include <steam_api.h>

#include <hidusage.h>
#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC ((USHORT)0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE ((USHORT)0x02)
#endif

int CALLBACK WinMain(_In_ HINSTANCE appInstance, _In_opt_ HINSTANCE preInstance, _In_ LPSTR cmdLine,
	_In_ int cmdCount) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Input::initilize(Renderer::getInstance()->getHandle());

	bool steamInit = SteamAPI_Init();

	ErrorLogger errorLogger;
	Input* input = Input::getInstance();
	Renderer* renderer = Renderer::getInstance();
	renderer->createCommonStates(); // if run on debug mode, game will crash if this line of code is run in Renderer Constructor!
	Settings::initialize();
	PathFindingThread* extraThread = PathFindingThread::getInstance();
	MSG msg = { 0 };

	StateStack stateStack;
	stateStack.push(StateItem::State::MainState);

	// high definition mouse input
	RAWINPUTDEVICE Rid[1];
	Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
	Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
	Rid[0].dwFlags = RIDEV_INPUTSINK;
	Rid[0].hwndTarget = renderer->getHandle();
	RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

	// random seed
	srand((unsigned int)time(NULL));

	while (stateStack.isEmpty() == false) {
		VariableSyncer::getInstance()->sync();
		input->update();
		AudioController::getInstance()->update();

		// start imgui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// Main loop
		stateStack.update();
		input->event_frameReset(); // reset accumulation of input events
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

	SteamAPI_Shutdown();

	Settings::getInstance()->saveAllSetting();
	VariableSyncer::getInstance()->saveAll();
	return 0;
}