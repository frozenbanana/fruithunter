#include "ErrorLogger.hpp"
#include <comdef.h>
#include <codecvt>


// This function works on a default console. Making colors and such. Tested on C++ online editor.
// TODO: Make it work on our console.
/*
enum EColor { BLACK = 30, RED = 31, GREEN = 32, YELLOW = 33, BLUE = 34, MAGENTA = 35, CYAN = 36, WHITE = 37 };
enum EStyle { NORMAL = 0, BOLD = 1, FAINT = 2, ITALIC = 3, UNDERLINE = 4, RIVERCED = 26, FRAMED = 51 };
void printFancy(const std::string str, EColor color, EStyle decoration = EStyle::NORMAL) {
	std::cout << "\033[" << decoration << ";" << color << "m" << str << "\033[0m" << std::endl;
}*/


std::string ws2s(const std::wstring& wstr) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

ErrorLogger::ErrorLogger() {
	AllocConsole();
	freopen_s(&m_filePtr, "CONOUT$", "w", stdout);
}

ErrorLogger::~ErrorLogger() { closeConsole(); }


void ErrorLogger::closeConsole() { fclose(m_filePtr); }

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
	// printFancy(output, YELLOW);
	std::cout << output;
}


void ErrorLogger::logError(HRESULT res, std::string message) {
	_com_error error(res);
	std::wstring resMessage = error.ErrorMessage();
	std::string output = message + " | " + ws2s(resMessage.c_str()) + "\n";
	// printFancy(output, RED);
	std::cout << output;
}

void ErrorLogger::log(std::string message) {
	// printFancy(message, WHITE);
	std::cout << message + "\n";
}
