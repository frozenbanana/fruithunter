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
	bool m_manualCamera = false;
	
	Sprite2D m_crosshair;

	MetricCollector m_metricCollector;
	Keyboard::Keys m_key_monitor = Keyboard::M;
	bool m_monitoring = false;

	//-- Private Functions --

public:
	static Scene* getScene();

	SceneManager();
	~SceneManager();

	void update(Camera* overrideCamera = nullptr);

	void setup_shadow(Camera* overrideCamera = nullptr);
	void draw_shadow();
	void setup_color(Camera* overrideCamera = nullptr);
	void draw_color();
	void draw_hud();

	void load(string folder);
	void reset();

	void monitor();

};
