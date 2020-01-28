#include "Window.hpp"
#include <Keyboard.h>
#include <Mouse.h>

LRESULT CALLBACK WinProc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_DESTROY || msg == WM_CLOSE) {
		PostQuitMessage(0);
		return 0;
	}

	switch (msg) {
	case WM_ACTIVATEAPP:
		DirectX::Keyboard::ProcessMessage(msg, wparam, lparam);
		DirectX::Mouse::ProcessMessage(msg, wparam, lparam);
		break;

	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		DirectX::Mouse::ProcessMessage(msg, wparam, lparam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(msg, wparam, lparam);
		break;
	}

	return DefWindowProc(handle, msg, wparam, lparam);
}

Window::Window(int width, int height) {
	// Define window style
	WNDCLASS wc = { 0 };
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WinProc;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.lpszClassName = L"DX11Tutorial";
	RegisterClass(&wc);

	// Create the window
	m_handle = CreateWindow(L"DX11Tutorial", L"C++11 and DX11 Tutorial",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE, 100, 100, width, height, nullptr, nullptr, nullptr, nullptr);
}

HWND Window::getHandle() { return m_handle; }
