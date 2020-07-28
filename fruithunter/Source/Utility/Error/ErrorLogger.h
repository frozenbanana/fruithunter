#pragma once
#include "GlobalNamespaces.h"
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
	static void logWarning(std::string message, HRESULT res);
	static void logWarning(std::string message);
	static void logError(std::string message, HRESULT res);
	static void logError(std::string message);
	static void log(std::string message);
	static void logFloat2(std::string description, float2 f2);
	static void logFloat3(std::string description, float3 f3);
	static void logInt2(std::string description, XMINT2 i2);

private:
	std::unique_ptr<FILE*> m_filePtr = std::make_unique<FILE*>();
};
