#pragma once
#include "GlobalNamespaces.h"
#include "TextRenderer.h"
#include <memory>
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <SpriteFont.h>

class HUD {
private:
	struct Sprite {
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
		float2 screenPos;
		float scale;
		int fruitType;
	};

	TextRenderer m_textRenderer;

	unique_ptr<SpriteBatch> m_spriteBatch;
	unique_ptr<CommonStates> m_states;

	vector<Sprite> m_sprites;

	float2 m_timerPos;
	float4 m_fruitTextColors[NR_OF_FRUITS];

	float m_secondsPassed = 0.0f;
	int m_minutesPassed = 0;
	int m_timeTargets[NR_OF_TIME_TARGETS];
	int m_inventory[NR_OF_FRUITS];

	string getMinutes();
	string getSeconds();
	void drawTargetTime();
	void setDepthStateToNull();

public:
	HUD();
	~HUD();

	void createFruitSprite(string fruitName);
	void setTimeTargets(int targets[]);
	void addFruit(int fruitType);
	void removeFruit(int fruitType);
	void update(float dt);
	void draw();
};