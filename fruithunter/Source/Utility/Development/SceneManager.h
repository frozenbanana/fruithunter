#pragma once
#include "Scene.h"
#include "HUD.h"

/*
 * Handles the drawing of the scene data
*/
class SceneManager {
protected:
	static shared_ptr<Scene> scene; //current active scene, is static so that other classes can fetch this information.
	HUD m_hud;
	bool m_manualCamera = false;
	string m_loadedScene = "";
	
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
	void restart();

};
