#pragma once
#include <Windows.h>
#include <iostream>

class ErrorLogger {
public:
	static void log(std::string message);
	static void log(HRESULT res, std::wstring message);
	static void log(HRESULT res, std::string message);
	/*
	static void log_warning(HRESULT res, std::string message);
	static void log_error(HRESULT res, std::string message);
	static void log(HRESULT res, std::string message);
	static void popUp(HRESULT res, std::string)
	*/
};
