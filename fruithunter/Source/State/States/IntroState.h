#pragma once
#include "State.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
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

	Button m_startButton;
	Button m_settingsButton;
	Button m_exitButton;

	Entity m_entity;

	// LOGO
	unique_ptr<SpriteBatch> m_spriteBatch;
	unique_ptr<CommonStates> m_states;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	float m_scale;
	float2 m_textureOffset;
};
