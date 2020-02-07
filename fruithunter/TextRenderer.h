#pragma once
#include "GlobalNamespaces.h"
#include <memory>
#include <DirectXMath.h>
#include <SpriteFont.h>


class TextRenderer {
public:
	TextRenderer();
	~TextRenderer();
	void draw(string text, float2 pos);
	void draw(string text, float2 pos, float4 col);

	void setColor(XMVECTORF32 color);

private:
	XMVECTORF32 m_color = Colors::White;
	std::unique_ptr<SpriteBatch> m_spriteBatch;
	std::unique_ptr<SpriteFont> m_spriteFont;
	float2 m_fontPos;

	void setDepthStateToNull();
};
