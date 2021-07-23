#pragma once
#include "StateItem.h"
#include "Timer.h"
#include "Entity.h"
#include "Button.h"
#include "Slider.h"
#include "Sprite2D.h"
#include "SceneManager.h"

#include "Menu_PoppingButton.h"
#include "Menu_Toggle.h"

class SettingsState : public StateItem {
private:
	Sprite2D m_settingsBackground;
	Button m_vsyncButton;
	Button m_FXAAButton;
	Button m_darkEdgesButton;
	Button m_shadowsButton;
	Button m_resolutionButton;

	Renderer::ScreenMode m_screenModeLocal = Renderer::ScreenMode::Screen_Windowed;
	Button m_screenMode;

	Button m_applyButton;
	Button m_backButton;

	Slider m_drawDistance;
	Slider m_masterVolume;
	Slider m_musicVolume;
	Slider m_effectsVolume;
	Slider m_sensitivity;

	bool m_redraw = false;
	SceneManager m_sceneManager; // used to redraw background

	bool m_screenStateChanged = false;

	static string screenModeToString(Renderer::ScreenMode mode);

public:
	SettingsState();
	~SettingsState();

	void init() override;
	void update(double dt) override;
	void draw() override;

	void pause() override;
	void play() override;
	void restart() override;

};
