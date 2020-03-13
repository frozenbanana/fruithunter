#pragma once
#include "State.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
#include <SpriteBatch.h>
#include <CommonStates.h>
#include "Timer.h"
#include <SpriteFont.h>
#define AMOUNT_OF_LETTERS 11

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
	Timer m_timer;
	// Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textures[AMOUNT_OF_LETTERS];
	wstring m_letterPaths[AMOUNT_OF_LETTERS];
	float2 m_letterPos[AMOUNT_OF_LETTERS];
	float2 m_speedOffsets[AMOUNT_OF_LETTERS];
	float m_scales[AMOUNT_OF_LETTERS];
	float2 m_textureOffsets[AMOUNT_OF_LETTERS];
};
