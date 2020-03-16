#pragma once
#include "State.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
#include "SkyBox.h"
#include "Camera.h"
#include "SeaEffect.h"
#include "EntityRepository.h"
#include "Timer.h"
#include "Apple.h"
#include "Bow.h"
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <SpriteFont.h>

class IntroState : public State {
public:
	IntroState();
	~IntroState();
	void update();
	void initialize();
	void handleEvent();
	void pause();
	void play();
	void draw();
	void createLogoSprite();
	void drawLogo();

private:
	TextRenderer m_textRenderer;

	Timer m_timer;

	Button m_startButton;
	Button m_settingsButton;
	Button m_exitButton;

	Entity m_entity;

	// Scene variables:
	Camera m_camera;
	SkyBox m_skybox;
	std::vector<string> m_maps;
	SeaEffect m_waterEffect;
	EntityRepository m_terrainProps;
	unique_ptr<Apple> m_apple;
	float m_totalDelta = 0.f;
	float m_shootThreshold = RandomFloat(2.4f, 4.f);
	float m_shootTime = 0.f;
	Bow m_bow;

	// LOGO
	unique_ptr<SpriteBatch> m_spriteBatch;
	unique_ptr<CommonStates> m_states;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	float m_scale;
	float2 m_textureOffset;
};
