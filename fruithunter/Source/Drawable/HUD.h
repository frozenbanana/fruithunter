#pragma once
#include "GlobalNamespaces.h"
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

	unique_ptr<SpriteBatch> m_spriteBatch;
	unique_ptr<SpriteFont> m_spriteFont;
	unique_ptr<CommonStates> m_states;

	vector<Sprite> m_sprites;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_backgroundTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_staminaTexture;

	float2 m_backgroundPos;
	float2 m_staminaPos;
	float2 m_timerPos;
	XMVECTORF32 m_fruitTextColors[NR_OF_FRUITS];

	float m_stamina = 1.0f;
	float m_secondsPassed = 0.0f;
	int m_minutesPassed = 0;
	bool m_victory = false;

	int m_timeTargets[NR_OF_TIME_TARGETS];
	int m_inventory[NR_OF_FRUITS];
	int m_winCondition[NR_OF_FRUITS];

	TimeTargets m_price = NR_OF_TIME_TARGETS;
	string getMinutes();
	string getSeconds();
	void drawTargetTime();
	void setDepthStateToNull();

public:
	HUD();
	~HUD();
	TimeTargets getPrize() const;
	string getTimePassed();
	bool hasWon();
	void createFruitSprite(string fruitName);
	void setTimeTargets(int targets[]);
	void setWinCondition(int winCons[]);
	void addFruit(int fruitType);
	void removeFruit(int fruitType);
	void update(float dt, float playerStamina);
	void draw();
};