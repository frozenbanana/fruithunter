#pragma once
#include <Windows.h>
#include <iostream>
#include <memory>

class ErrorLogger {
public:
	ErrorLogger();
	~ErrorLogger();
	void closeConsole();
	static void messageBox(std::string message);
	static void messageBox(HRESULT res, std::wstring message);
	static void messageBox(HRESULT res, std::string message);
	static void logWarning(HRESULT res, std::string message);
	static void logError(HRESULT res, std::string message);
	static void log(std::string message);

private:
	std::unique_ptr<FILE*> m_filePtr = std::make_unique<FILE*>();
};
