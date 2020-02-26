#include "HUD.h"
#include "Renderer.h"
#include "WICTextureLoader.h"
#include "ErrorLogger.h"

#include <iomanip>
#include <sstream>

string HUD::getMinutes() {
	if (m_secondsPassed > 60.0f) {
		m_secondsPassed -= 60.0f;
		m_minutesPassed++;
	}

	return (m_minutesPassed < 10 ? "0" : "") + to_string(m_minutesPassed);
}

string HUD::getSeconds() {
	stringstream tempStream;
	tempStream << std::fixed << setprecision(2) << m_secondsPassed;
	string seconds = tempStream.str();

	return (m_secondsPassed < 10.0f ? "0" : "") + seconds;
}

void HUD::drawTargetTime() {
	float timePassed = m_minutesPassed * 60.0f + m_secondsPassed;

	int goldTarget = m_timeTargets[GOLD];
	int silverTarget = m_timeTargets[SILVER];
	int bronzeTarget = m_timeTargets[BRONZE];

	string timeString;
	float4 color;

	if (timePassed < goldTarget) {
		timeString = "Target: 0" + to_string(goldTarget / 60) + ":" +
					 (goldTarget % 60 < 10 ? "0" : "") + to_string(goldTarget % 60) + ".00";
		color = float4(1.0f, 0.85f, 0.0f, 1.0f);
	}
	else if (timePassed < silverTarget) {
		timeString = "Target: 0" + to_string(silverTarget / 60) + ":" +
					 (silverTarget % 60 < 10 ? "0" : "") + to_string(silverTarget % 60) + ".00";
		color = float4(0.8f, 0.8f, 0.8f, 1.0f);
	}
	else if (timePassed < bronzeTarget) {
		timeString = "Target: 0" + to_string(bronzeTarget / 60) + ":" +
					 (bronzeTarget % 60 < 10 ? "0" : "") + to_string(bronzeTarget % 60) + ".00";
		color = float4(0.85f, 0.55f, 0.25f, 1.0f);
	}
	else {
		timeString = "Target: 0" + to_string(bronzeTarget / 60) + ":" +
					 (bronzeTarget % 60 < 10 ? "0" : "") + to_string(bronzeTarget % 60) + ".00";
		color = float4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	wstring wText = std::wstring(timeString.begin(), timeString.end());
	m_spriteFont->DrawString(
		m_spriteBatch.get(), wText.c_str(), float2(25.0f, STANDARD_HEIGHT - 150.0f), color);
}

void HUD::setDepthStateToNull() {
	Renderer::getInstance()->getDeviceContext()->OMSetDepthStencilState(nullptr, 0);
}

HUD::HUD() {
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(
		Renderer::getDevice(), L"assets/fonts/myfile.spritefont");

	if (!m_spriteFont.get()) {
		ErrorLogger::log("HUD failed to load font.");
		return;
	}

	m_fruitTextColors[APPLE] = { 1.f, 0.f, 0.f, 1.f };
	m_fruitTextColors[BANANA] = { 0.9f, 0.7f, 0.2f, 1.f };
	m_fruitTextColors[MELON] = { 0.4f, 0.7f, 0.3f, 1.f };

	m_spriteBatch = std::make_unique<SpriteBatch>(Renderer::getDeviceContext());
	m_states = std::make_unique<CommonStates>(Renderer::getDevice());

	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	HRESULT t = CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/background.png",
		resource.GetAddressOf(), m_backgroundTexture.ReleaseAndGetAddressOf());

	if (t)
		ErrorLogger::logError(t, "Failed to create sprite texture");

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	resource.As(&tex);
	CD3D11_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

	m_backgroundPos = float2(15.0f, STANDARD_HEIGHT - 150.0f);
}

HUD::~HUD() {
	m_spriteBatch.reset();
	m_states.reset();
}

void HUD::createFruitSprite(string fruitName) {
	Sprite sprite;
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	wstring fileName = wstring(fruitName.begin(), fruitName.end());
	wstring filePath = L"assets/sprites/" + fileName + L".png";

	HRESULT t = CreateWICTextureFromFile(Renderer::getDevice(), filePath.c_str(),
		resource.GetAddressOf(), sprite.texture.ReleaseAndGetAddressOf());

	if (t)
		ErrorLogger::logError(t, "Failed to create sprite texture");

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	resource.As(&tex);
	CD3D11_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

	if (fruitName == "apple")
		sprite.fruitType = APPLE;
	else if (fruitName == "banana")
		sprite.fruitType = BANANA;
	else if (fruitName == "melon")
		sprite.fruitType = MELON;

	sprite.scale = 75.0f / (float)texDesc.Height;
	sprite.screenPos.x = 25.0f;
	sprite.screenPos.y = 25.0f + 100.0f * m_sprites.size();

	m_sprites.push_back(sprite);
}

void HUD::setTimeTargets(int targets[]) {
	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
		m_timeTargets[i] = targets[i];
	}
}

void HUD::addFruit(int fruitType) { m_inventory[fruitType]++; }

void HUD::removeFruit(int fruitType) { m_inventory[fruitType]--; }

void HUD::update(float dt) { m_secondsPassed += dt; }

void HUD::draw() {
	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

	for (size_t i = 0; i < m_sprites.size(); i++) {
		m_spriteBatch->Draw(m_sprites[i].texture.Get(), m_sprites[i].screenPos, nullptr,
			Colors::White, 0.f, float2(0.0f, 0.0f), m_sprites[i].scale);
	}

	m_spriteBatch->Draw(m_backgroundTexture.Get(), m_backgroundPos);

	m_spriteBatch->End();

	m_spriteBatch->Begin();

	string timeString = "  Time: " + getMinutes() + ":" + getSeconds();
	wstring wText = wstring(timeString.begin(), timeString.end());

	m_spriteFont->DrawString(
		m_spriteBatch.get(), wText.c_str(), float2(25.0f, STANDARD_HEIGHT - 100.0f));
	drawTargetTime();

	for (size_t i = 0; i < m_sprites.size(); i++) {
		wText = to_wstring(m_inventory[m_sprites[i].fruitType]);
		m_spriteFont->DrawString(m_spriteBatch.get(), wText.c_str(),
			m_sprites[i].screenPos + float2(75.0f, 0.0f),
			m_fruitTextColors[m_sprites[i].fruitType]);
	}

	m_spriteBatch->End();

	setDepthStateToNull();
}
