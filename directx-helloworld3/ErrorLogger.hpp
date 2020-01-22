#pragma once
#include <Windows.h>
#include <iostream>

class ErrorLogger {
public:
	static void log(std::string message);
	static void log(HRESULT res, std::wstring message);
	static void log(HRESULT res, std::string message);
};
