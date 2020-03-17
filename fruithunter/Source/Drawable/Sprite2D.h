#pragma once
#include "GlobalNamespaces.h"
#include <WICTextureLoader.h>
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <SpriteFont.h>
class Sprite2D {
private:
	static unique_ptr<SpriteBatch> m_spriteBatch;
	static unique_ptr<CommonStates> m_states;
	const string m_prepath = "assets/sprites/";

	bool m_initilized = false;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
	string m_path = "";
	float2 m_position = float2(0, 0);
	float m_scale = 0.25f;
	float m_rotation = 0.0f;
	XMINT2 m_textureSize = XMINT2(0, 0);

public:
	XMINT2 getTextureSize() const;
	void setPosition(float2 position);
	void setScale(float scale);
	void setRotation(float rot);
	void draw();

	bool load(string path);
	Sprite2D();
	~Sprite2D();
};
