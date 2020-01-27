#pragma once
#include <Windows.h>
#include <iostream>

class ErrorLogger {
public:
	static void messageBox(std::string message);
	static void messageBox(HRESULT res, std::wstring message);
	static void messageBox(HRESULT res, std::string message);
	/*
	static void logWarning(HRESULT res, std::string message);
	static void logError(HRESULT res, std::string message);
	static void log(HRESULT res, std::string message);
	static void popUp(HRESULT res, std::string)
	*/
};
