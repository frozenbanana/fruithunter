#include "SteamAPICommunicator.h"
#include "ErrorLogger.h"

SteamAPICommunicator SteamAPICommunicator::m_this;

SteamAPICommunicator* SteamAPICommunicator::getInstance() { return &m_this; }

bool SteamAPICommunicator::init() {
	if (!SteamAPI_Init()) {
		ErrorLogger::logError(
			"Fatal Error - Steam must be running to play this game (SteamAPI_Init() failed).");
		return false;
	}

	return true;
}

void SteamAPICommunicator::shutdown() { SteamAPI_Shutdown(); }

SteamAPICommunicator::SteamAPICommunicator() {}
