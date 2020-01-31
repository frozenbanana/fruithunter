#pragma once
#include <memory>
#include <SpriteFont.h>
#include <SimpleMath.h>

class TextRenderer {
public:
	TextRenderer();
	void draw();

private:
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
	DirectX::SimpleMath::Vector2 m_fontPos;
};
