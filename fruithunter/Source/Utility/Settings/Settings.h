#pragma once

class Settings {
private:
	Settings();
	static Settings m_this;

	bool m_vsync = true;
	bool m_darkEdges = true;
	bool m_particles = true;

	float m_masterVolume = 1.0f;
	float m_musicVolume = 0.5f;
	float m_effectsVolume = 0.5f;

public:
	static void initialize();
	static Settings* getInstance();
	~Settings();

	void setVsync(bool value);
	void setDarkEdges(bool value);
	void setParticles(bool value);

	void setMasterVolume(float value);
	void setMusicVolume(float value);
	void setEffectsVolume(float value);

	bool getVsync();
	bool getDarkEdges();
	bool getParticles();

	float getMasterVolume();
	float getMusicVolume();
	float getEffectsVolume();
};