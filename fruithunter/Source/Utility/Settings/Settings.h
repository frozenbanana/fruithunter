#pragma once
#include "GlobalNamespaces.h"

class Settings {
private:
	Settings();
	static Settings m_this;

	const string m_filePath = "assets/settings.txt";

	bool m_vsync = true;
	bool m_fullscreen = false;
	bool m_darkEdges = true;

	float m_masterVolume = 0.5f;
	float m_musicVolume = 0.5f;
	float m_effectsVolume = 0.5f;
	float m_drawDistance = 0.5f;

	XMINT2 m_resolution = XMINT2(1280, 720);
	XMINT2 m_shadowResolution = XMINT2(2048, 2048);

	string getSetting(ifstream* input);

public:
	static void initialize();
	static Settings* getInstance();
	~Settings();

	void loadAllSetting();
	void saveAllSetting();

	void setVsync(bool value);
	void setDarkEdges(bool value);
	void setFullscreen(bool value);

	void setDrawDistance(float value);
	void setMasterVolume(float value);
	void setMusicVolume(float value);
	void setEffectsVolume(float value);

	void setResolution(int width, int height);
	void setShadowResolution(int res);

	bool getVsync();
	bool getDarkEdges();
	bool getFullscreen();

	float getDrawDistance();
	float getMasterVolume();
	float getMusicVolume();
	float getEffectsVolume();

	XMINT2 getResolution();
	XMINT2 getShadowResolution();
};