#include "TextRenderer.h"
#include "Renderer.h"
#include "ErrorLogger.h"

TextRenderer::TextRenderer() {
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(
		Renderer::getDevice(), L"assets/fonts/myfile.spritefont");

	if (!m_spriteFont.get()) {
		ErrorLogger::log("TextRenderer failed to load font.");
		return;
	}
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(Renderer::getDeviceContext());

	m_fontPos.x = (float)STANDARD_WIDTH / 2.f;
	m_fontPos.y = (float)STANDARD_HEIGHT / 2.f;
}

TextRenderer::~TextRenderer() { m_spriteBatch.reset(); }


void TextRenderer::setColor(DirectX::XMVECTORF32 color) { m_color = color; }

void TextRenderer::setDepthStateToNull() {
	/*
		spritebatch anv�nder sin egen eller f�r�ndrar p� n�got vis dpthstencilstate, vi har ingen
	   s�nn s� sb "sl�r av" v�ran depthbuffer, s�tt state till nullptr f�r att sl� p� v�r db igen
		kalla p� denna i alla textrenderarens drawfunctioner.
	*/
	Renderer::getInstance()->getDeviceContext()->OMSetDepthStencilState(nullptr, 0);
}


// By default SpriteBatch uses premultiplied alpha blending, no depth buffer,
// counter clockwise culling, and linear filtering with clamp texture addressing.
// You can change this by passing custom state objects to SpriteBatch::Begin
// See: https://github.com/microsoft/DirectXTK/wiki/SpriteBatch
void TextRenderer::draw(std::string text, float2 pos) {
	m_spriteBatch->Begin(DirectX::SpriteSortMode_BackToFront);
	std::wstring wText = std::wstring(text.begin(), text.end());

	Vector2 origin = Vector2(m_spriteFont->MeasureString(wText.c_str())) / 2.0f;

	m_spriteFont->DrawString(m_spriteBatch.get(), wText.c_str(), pos, m_color, 0.f, origin);

	m_spriteBatch->End();
	setDepthStateToNull();
}

void TextRenderer::draw(std::string text, float2 pos, float4 col) {
	m_spriteBatch->Begin();
	std::wstring wText = std::wstring(text.begin(), text.end());

	float2 origin = float2(m_spriteFont->MeasureString(wText.c_str())) / 2.0f;

	DirectX::XMVECTORF32 _col = { col.x, col.y, col.z, col.w };

	m_spriteFont->DrawString(m_spriteBatch.get(), wText.c_str(), pos, _col, 0.f, origin);

	m_spriteBatch->End();
	setDepthStateToNull();
}