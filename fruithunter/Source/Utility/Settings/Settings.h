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
	struct SettingContainer {
		bool vsync = false;
		bool fullscreen = false;
		bool darkEdges = true;
		float volume_master = 0.5f;
		float volume_music = 0.5f;
		float volume_effect = 0.5f;
		float drawDistance = 0.5f;
		float sensitivity = 0.5f;
		XMINT2 resolution = XMINT2(1280,720);
		int resolution_shadow = 2048;
	} m_tempContainer, m_container; // need to copy each value manually to ensure overhead(other classes) is updated correctly

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