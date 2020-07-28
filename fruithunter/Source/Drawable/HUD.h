#pragma once
#include "GlobalNamespaces.h"
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <SpriteFont.h>

class HUD {
private:
	struct Sprite {
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
		float2 screenPos = float2(0, 0);
		float scale = 1;
		FruitType fruitType = FruitType::APPLE;
		float pickUp = 0; // scales fruit sprite when fruit is picked up. 
	};

	unique_ptr<SpriteBatch> m_spriteBatch;
	unique_ptr<SpriteFont> m_spriteFont;
	unique_ptr<CommonStates> m_states;

	//vector<Sprite> m_sprites;

	struct OnScreenFruitItem {
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture;
		float scale = 1;
		float extraScaling = 0; // scales fruit sprite when fruit is picked up. 
		float4 textColor = float4(1.);
	};
	OnScreenFruitItem m_fruitSprites[NR_OF_FRUITS];

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_backgroundTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_staminaTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_staminaFrame;

	float2 m_backgroundPos;
	float2 m_staminaPos;
	float2 m_timerPos;
	XMVECTORF32 m_fruitTextColors[NR_OF_FRUITS];

	//-- Private Functions --

	void initilizeFruitItem(OnScreenFruitItem& item, string imageFilename, float4 textColor);

	void drawTargetTime();

	void setDepthStateToNull();

public:
	HUD();
	~HUD();

	void draw();
};