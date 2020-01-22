#include "ErrorLogger.hpp"
#include <comdef.h>

void ErrorLogger::log(std::string message) {
	std::string error_message = "Error: " + message;
	MessageBoxA(NULL, error_message.c_str(), "Error", MB_ICONERROR);
}

void ErrorLogger::log(HRESULT hr, std::string message) {
	_com_error error(hr);
	std::wstring convertedMessage = std::wstring(message.begin(), message.end());
	std::wstring error_message = L"Error: " + convertedMessage + L"\n" + error.ErrorMessage();
	MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR);
}

void ErrorLogger::log(HRESULT hr, std::wstring message) {
	_com_error error(hr);
	std::wstring error_message = L"Error: " + message + L"\n" + error.ErrorMessage();
	MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR);
}
