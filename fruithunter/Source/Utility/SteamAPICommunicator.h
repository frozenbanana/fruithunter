#pragma once
#include <steam_api.h>

class SteamAPICommunicator {
private:
	static SteamAPICommunicator m_this;

	SteamAPICommunicator();

public:
	static SteamAPICommunicator* getInstance();
	// General
	bool init();
	void shutdown();
};
