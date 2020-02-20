#include "ErrorLogger.h"
#include <iostream>
#include <comdef.h>
#include <codecvt>

std::string ws2s(const std::wstring& wstr) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

ErrorLogger::ErrorLogger() {
	AllocConsole();
	freopen_s(m_filePtr.get(), "CONOUT$", "w", stdout);
}

ErrorLogger::~ErrorLogger() { closeConsole(); }


void ErrorLogger::closeConsole() { fclose(*m_filePtr); }

void ErrorLogger::messageBox(std::string message) {
	std::string errorMessage = "Error: " + message;
	MessageBoxA(NULL, errorMessage.c_str(), "Error", MB_ICONERROR);
}

void ErrorLogger::messageBox(HRESULT res, std::string message) {
	_com_error error(res);
	std::wstring convertedMessage = std::wstring(message.begin(), message.end());
	std::wstring errorMessage = L"Error: " + convertedMessage + L"\n" + error.ErrorMessage();
	MessageBoxW(NULL, errorMessage.c_str(), L"Error", MB_ICONERROR);
}

void ErrorLogger::messageBox(HRESULT res, std::wstring message) {
	_com_error error(res);
	std::wstring errorMessage = L"Error: " + message + L"\n" + error.ErrorMessage();
	MessageBoxW(NULL, errorMessage.c_str(), L"Error", MB_ICONERROR);
}

void ErrorLogger::logWarning(HRESULT res, std::string message) {
	_com_error error(res);
	std::wstring resMessage = error.ErrorMessage();
	std::string output = message + " | " + ws2s(resMessage.c_str()) + "\n";

	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console, 6); // 6 = Yellow
	std::cout << output;
	SetConsoleTextAttribute(console, 15); // 15 = White
}

void ErrorLogger::logError(HRESULT res, std::string message) {
	_com_error error(res);
	std::wstring resMessage = error.ErrorMessage();
	std::string output = message + " | " + ws2s(resMessage.c_str()) + "\n";

	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(console, 4); // 4 = Red
	std::cout << output;
	SetConsoleTextAttribute(console, 15); // 15 = White
}

void ErrorLogger::log(std::string message) { std::cout << message + "\n"; }

void ErrorLogger::logFloat3(std::string description, float3 f3) {
	std::cout << description + " : " + std::to_string(f3.x) + ", " + std::to_string(f3.y) + ", " +
					 std::to_string(f3.z) + "\n";
}
