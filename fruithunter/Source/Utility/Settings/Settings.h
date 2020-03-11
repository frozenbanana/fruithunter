#pragma once

class Settings {
private:
	Settings();
	static Settings m_this;

	bool m_vsync = true;
	bool m_darkEdges = true;
	bool m_particles = true;

public:
	static void initialize();
	static Settings* getInstance();
	~Settings();

	void setVsync(bool value);
	void setDarkEdges(bool value);
	void setParticles(bool value);

	bool getVsync();
	bool getDarkEdges();
	bool getParticles();
};