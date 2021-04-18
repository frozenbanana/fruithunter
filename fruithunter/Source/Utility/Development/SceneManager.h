#pragma once
#include "Scene.h"
#include "HUD.h"
#include "Sprite2D.h"
#include "MetricCollector.h"
/*
 * Handles the drawing of the scene data
*/
class SceneManager {
protected:
	static shared_ptr<Scene> scene; //current active scene, is static so that other classes can fetch this information.
	HUD m_hud;
	bool m_playerState = true;
	bool m_fotoMode = false;
	
	Sprite2D m_crosshair;

	MetricCollector m_metricCollector;
	Keyboard::Keys m_key_monitor = Keyboard::M;
	Keyboard::Keys m_key_fotoMode = Keyboard::N;
	bool m_monitoring = false;

	//-- Private Functions --

public:
	static Scene* getScene();

	void setPlayerState(bool state);
	bool getPlayerState() const;
	void setFotoMode(bool state);
	bool getFotoMode() const;

	SceneManager();
	~SceneManager();

	void update();

	void setup_shadow();
	void draw_shadow();
	void setup_color();
	void draw_color();
	void draw_hud();

	void load(string folder);
	void reset();

	void monitor();

};
