#include <Windows.h>
#include "Window.hpp"
#include "Renderer.hpp"
#include "Quad.hpp"

int CALLBACK WinMain(_In_ HINSTANCE appInstance,
	_In_opt_ HINSTANCE preInstance, 
	_In_ LPSTR cmdLine, 
	_In_ int cmdCount) {

	Window window(800, 600);
	Renderer renderer(window);
	Quad quad(renderer);

	MSG msg = { 0 };
	while (true) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT) {
				break;
			}

			// Main loop
			renderer.beginFrame();
			quad.draw(renderer);
			renderer.endFrame();
		}
	}
	
	return 0;
}