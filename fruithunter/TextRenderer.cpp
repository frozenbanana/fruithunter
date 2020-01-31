#include "TextRenderer.hpp"
#include "Renderer.hpp"
#include <SimpleMath.h>

using Vector2 = DirectX::SimpleMath::Vector2;
using Vector4 = DirectX::SimpleMath::Vector4;

TextRenderer::TextRenderer() {
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(
		Renderer::getDevice(), L"assets/fonts/myfile.spritefont");

	if (!m_spriteFont.get()) {
		ErrorLogger::log("TextRenderer failed to load font.");
		return;
	}

	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(Renderer::getDeviceContext());

	m_fontPos.x = 800 / 2.f;
	m_fontPos.y = 600 / 2.f;
}

TextRenderer::~TextRenderer() { m_spriteBatch.reset(); }


void TextRenderer::setColor(DirectX::XMVECTORF32 color) { m_color = color; }


// By default SpriteBatch uses premultiplied alpha blending, no depth buffer,
// counter clockwise culling, and linear filtering with clamp texture addressing.
// You can change this by passing custom state objects to SpriteBatch::Begin
// See: https://github.com/microsoft/DirectXTK/wiki/SpriteBatch
void TextRenderer::draw(std::string text, Vector2 pos) {
	m_spriteBatch->Begin();
	std::wstring wText = std::wstring(text.begin(), text.end());

	Vector2 origin = Vector2(m_spriteFont->MeasureString(wText.c_str())) / 2.0f;

	m_spriteFont->DrawString(m_spriteBatch.get(), wText.c_str(), pos, m_color, 0.f, origin);

	m_spriteBatch->End();
}

void TextRenderer::draw(std::string text, Vector2 pos, Vector4 col) {
	m_spriteBatch->Begin();
	std::wstring wText = std::wstring(text.begin(), text.end());

	Vector2 origin = Vector2(m_spriteFont->MeasureString(wText.c_str())) / 2.0f;

	DirectX::XMVECTORF32 _col = { col.x, col.y, col.z, col.w };

	m_spriteFont->DrawString(m_spriteBatch.get(), wText.c_str(), pos, _col, 0.f, origin);

	m_spriteBatch->End();
}