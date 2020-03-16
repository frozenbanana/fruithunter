#include "HUD.h"
#include "Renderer.h"
#include "WICTextureLoader.h"
#include "ErrorLogger.h"

#include <iomanip>
#include <sstream>

string HUD::getTimePassed() { return getMinutes() + ":" + getSeconds(); }

string HUD::getMinutes() {
	if (m_secondsPassed > 60.0f) {
		m_secondsPassed -= 60.0f;
		m_minutesPassed++;
	}

	return (m_minutesPassed < 10 ? "0" : "") + to_string(m_minutesPassed);
}

string HUD::getSeconds() {
	// Float to string with 2 decimals
	stringstream tempStream;
	tempStream << std::fixed << setprecision(2) << m_secondsPassed;
	string seconds = tempStream.str();

	return (m_secondsPassed < 10.0f ? "0" : "") + seconds;
}

TimeTargets HUD::getPrize() const { return m_price; }

void HUD::drawTargetTime() {
	// Get time passed in seconds
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
		m_price = GOLD;
	}
	else if (timePassed < silverTarget) {
		timeString = "Target: 0" + to_string(silverTarget / 60) + ":" +
			(silverTarget % 60 < 10 ? "0" : "") + to_string(silverTarget % 60) + ".00";
		color = float4(0.8f, 0.8f, 0.8f, 1.0f);
		m_price = SILVER;
	}
	else if (timePassed < bronzeTarget) {
		timeString = "Target: 0" + to_string(bronzeTarget / 60) + ":" +
			(bronzeTarget % 60 < 10 ? "0" : "") + to_string(bronzeTarget % 60) + ".00";
		color = float4(0.85f, 0.55f, 0.25f, 1.0f);
		m_price = BRONZE;
	}
	else {
		timeString = "Target: 0" + to_string(bronzeTarget / 60) + ":" +
			(bronzeTarget % 60 < 10 ? "0" : "") + to_string(bronzeTarget % 60) + ".00";
		color = float4(1.0f, 0.0f, 0.0f, 1.0f);
		m_price = NR_OF_TIME_TARGETS;
	}

	wstring wText = std::wstring(timeString.begin(), timeString.end());
	m_spriteFont->DrawString(
		m_spriteBatch.get(), wText.c_str(), float2(25.0f, SCREEN_HEIGHT - 150.0f), color);
}

void HUD::setDepthStateToNull() {
	Renderer::getInstance()->getDeviceContext()->OMSetDepthStencilState(nullptr, 0);
}

HUD::HUD() {
	// Text font
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(
		Renderer::getDevice(), L"assets/fonts/myfile2.spritefont");

	if (!m_spriteFont.get()) {
		ErrorLogger::log("HUD failed to load font.");
		return;
	}

	// Text colors for fruit inventory
	m_fruitTextColors[APPLE] = { 1.f, 0.f, 0.f, 1.f };
	m_fruitTextColors[BANANA] = { 0.9f, 0.7f, 0.2f, 1.f };
	m_fruitTextColors[MELON] = { 0.4f, 0.7f, 0.3f, 1.f };

	m_spriteBatch = std::make_unique<SpriteBatch>(Renderer::getDeviceContext());
	m_states = std::make_unique<CommonStates>(Renderer::getDevice());

	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	HRESULT t = CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/background.png",
		resource.GetAddressOf(), m_backgroundTexture.ReleaseAndGetAddressOf());
	if (t)
		ErrorLogger::logError(t, "Failed to create backgorund sprite texture");
	m_backgroundPos = float2(15.0f, SCREEN_HEIGHT - 150.0f);

	t = CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/stamina.png",
		resource.GetAddressOf(), m_staminaTexture.ReleaseAndGetAddressOf());
	if (t)
		ErrorLogger::logError(t, "Failed to create stamina sprite texture");
	m_staminaPos = float2(SCREEN_WIDTH - 250.0f, SCREEN_HEIGHT - 100.0f);

	t = CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/staminaFrame.png",
		resource.GetAddressOf(), m_staminaFrame.ReleaseAndGetAddressOf());
	if (t)
		ErrorLogger::logError(t, "Failed to create stamina frame texture");
}

HUD::~HUD() {
	m_spriteBatch.reset();
	m_states.reset();
}

bool HUD::hasWon() { return m_victory; }

void HUD::createFruitSprite(string fruitName) {
	Sprite sprite;
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	wstring fileName = wstring(fruitName.begin(), fruitName.end());
	wstring filePath = L"assets/sprites/" + fileName + L".png";

	HRESULT t = CreateWICTextureFromFile(Renderer::getDevice(), filePath.c_str(),
		resource.GetAddressOf(), sprite.texture.ReleaseAndGetAddressOf());

	if (t)
		ErrorLogger::logError(t, "Failed to create fruit sprite texture");

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

	// Set all sprites to the same size with equal spacing
	sprite.scale = 75.0f / (float)texDesc.Height;
	sprite.screenPos.x = 25.0f;
	sprite.screenPos.y = 25.0f + 100.0f * m_sprites.size();
	sprite.pickUp = 0.f;

	m_sprites.push_back(sprite);
}

void HUD::setTimeTargets(int targets[]) {
	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
		m_timeTargets[i] = targets[i];
	}
}

void HUD::setWinCondition(int winCons[]) {
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		m_winCondition[i] = winCons[i];
	}
}

void HUD::addFruit(int fruitType) {
	m_inventory[fruitType]++;
	for (size_t i = 0; i < m_sprites.size(); i++) {
		if (m_sprites[i].fruitType == fruitType)
			m_sprites[i].pickUp = 0.5f;
	}

	bool completed = true;
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		if (m_inventory[i] < m_winCondition[i])
			completed = false;
	}

	if (completed)
		m_victory = true;
}

void HUD::removeFruit(int fruitType) { m_inventory[fruitType]--; }

void HUD::update(float dt, float playerStamina) {
	m_stamina = playerStamina;

	for (size_t i = 0; i < m_sprites.size(); i++) {
		m_sprites[i].pickUp = max(0.f, m_sprites[i].pickUp - dt);
	}

	if (!m_victory)
		m_secondsPassed += dt;
}

void HUD::draw() {
	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

	// Draw fruit icons
	for (size_t i = 0; i < m_sprites.size(); i++) {
		m_spriteBatch->Draw(m_sprites[i].texture.Get(), m_sprites[i].screenPos, nullptr,
			Colors::White, 0.f, float2(0.0f, 0.0f),
			m_sprites[i].scale + 0.1f * m_sprites[i].pickUp);
	}

	// Draw text background
	m_spriteBatch->Draw(m_backgroundTexture.Get(), m_backgroundPos);
	m_spriteBatch->Draw(m_staminaTexture.Get(), m_staminaPos, nullptr, Colors::White, 0.0f,
		float2(0.0f, 0.0f), float2(m_stamina + 0.05f, 0.8f));
	m_spriteBatch->Draw(m_staminaFrame.Get(),
		float2(m_staminaPos.x - 27.0f, m_staminaPos.y - 10.0f), nullptr, Colors::White, 0.0f,
		float2(0.0f, 0.0f), float2(1.05f, 0.8f));

	m_spriteBatch->End();

	m_spriteBatch->Begin();

	string timeString = "  Time: " + getMinutes() + ":" + getSeconds();
	wstring wText = wstring(timeString.begin(), timeString.end());

	// Draw time and target time
	m_spriteFont->DrawString(
		m_spriteBatch.get(), wText.c_str(), float2(25.0f, SCREEN_HEIGHT - 100.0f));
	drawTargetTime();

	// Draw inventory numbers
	for (size_t i = 0; i < m_sprites.size(); i++) {
		wText = to_wstring(m_inventory[m_sprites[i].fruitType]) + L"/" +
			to_wstring(m_winCondition[m_sprites[i].fruitType]);
		m_spriteFont->DrawString(m_spriteBatch.get(), wText.c_str(),
			m_sprites[i].screenPos + float2(75.0f, 0.0f),
			m_fruitTextColors[m_sprites[i].fruitType]);
	}

	m_spriteBatch->End();

	// Reset depth state
	setDepthStateToNull();
}
