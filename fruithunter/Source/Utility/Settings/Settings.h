#pragma once
#include "GlobalNamespaces.h"
#include "VariableSyncer.h"

class Settings {
private:
	Settings();
	static Settings m_this;

	FileSyncer m_settingFile;

	const string m_path = "assets/settings.txt";

	// settings variables
	bool m_vsync = true;
	bool m_fullscreen = false;
	bool m_darkEdges = true;

	float m_masterVolume = 0.5f;
	float m_musicVolume = 0.5f;
	float m_effectsVolume = 0.5f;
	float m_drawDistance = 0.5f;
	float m_sensitivity = 0.5f;

	XMINT2 m_resolution = XMINT2(1280, 720);
	int m_shadowResolution = 2048;

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
	void setSensitivity(float value);

	void setResolution(int width, int height);
	void setShadowResolution(int res);

	bool getVsync();
	bool getDarkEdges();
	bool getFullscreen();

	float getDrawDistance();
	float getDrawDistanceValue();
	float getMasterVolume();
	float getMusicVolume();
	float getEffectsVolume();
	float getSensitivity();

	XMINT2 getResolution();
	XMINT2 getShadowResolution();
};