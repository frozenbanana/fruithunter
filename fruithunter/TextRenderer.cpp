#include "TextRenderer.hpp"
#include "Renderer.hpp"
#include <SimpleMath.h>

using Vector2 = DirectX::SimpleMath::Vector2;

TextRenderer::TextRenderer() {
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(
		Renderer::getDevice(), L"assets/fonts/myfile.spritefont");

	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(Renderer::getDeviceContext());


	m_fontPos.x = 800 / 2.f;
	m_fontPos.y = 600 / 2.f;
}

void TextRenderer::draw() {
	m_spriteBatch->Begin();

	const wchar_t* output = L"Hello World";

	Vector2 origin = Vector2(m_spriteFont->MeasureString(output)) / 2.f;

	m_spriteFont->DrawString(
		m_spriteBatch.get(), output, m_fontPos, DirectX::Colors::White, 0.f, origin);

	m_spriteBatch->End();
}
