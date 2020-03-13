#pragma once
#include "GlobalNamespaces.h"
#include <WICTextureLoader.h>
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <SpriteFont.h>
class LogoLetter {
private:
	static unique_ptr<SpriteBatch> m_spriteBatch;
	static unique_ptr<CommonStates> m_states;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
	string m_letterPath;
	float2 m_letterPos;
	float2 m_speedOffset;
	float m_scale = 0.25f;
	XMINT2 m_textureSize;

public:
	XMINT2 getTextureSize()const;
	void setPosition(float2 position);

	void update(float timePassed);

	void draw();

	void load(string path);
	LogoLetter();
	~LogoLetter();
};
