#pragma once
#include <memory>
#include <DirectXMath.h>
#include <SpriteFont.h>
#include <SimpleMath.h>
#include <iostream>


using Vector2 = DirectX::SimpleMath::Vector2;
using Vector4 = DirectX::SimpleMath::Vector4;

class TextRenderer {
public:
	TextRenderer();
	~TextRenderer();
	void draw(std::string text, Vector2 pos);
	void draw(std::string text, Vector2 pos, Vector4 col);

	void setColor(DirectX::XMVECTORF32 color);

private:
	DirectX::XMVECTORF32 m_color = DirectX::Colors::White;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::SpriteFont> m_spriteFont;
	DirectX::SimpleMath::Vector2 m_fontPos;

	void setDepthStateToNull();
};
