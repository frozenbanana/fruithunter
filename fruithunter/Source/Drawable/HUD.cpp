#include "HUD.h"
#include "Renderer.h"
#include "WICTextureLoader.h"
#include "ErrorLogger.h"
#include "SceneManager.h"

void HUD::drawTargetTime() {
	// Get time passed in seconds
	float timePassed = SceneManager::getScene()->m_timer.getTimePassed();

	int* timeTargets = SceneManager::getScene()->m_utility.timeTargets;
	//find index of achieved target
	int index = NR_OF_TIME_TARGETS; // holds index for timeTargets. If no targetTime is achieved then nr_of_targets is defined as null. 
	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
		if (timePassed < timeTargets[i]) {
			index = i;
			break;
		}
	}
	//set color and time target
	float4 targetColors[NR_OF_TIME_TARGETS + 1] = {
		float4(1.0f, 0.85f, 0.0f, 1.0f),   // gold
		float4(0.8f, 0.8f, 0.8f, 1.0f),	   // silver
		float4(0.85f, 0.55f, 0.25f, 1.0f), // bronze
		float4(1.0f, 0.0f, 0.0f, 1.0f)	   // no target (color)
	};
	float4 color = targetColors[index];
	int target = timeTargets[BRONZE]; // no target (time)
	if (index != NR_OF_TIME_TARGETS)
		target = timeTargets[index];
	string timeString = "Target: " + Time2DisplayableString(target) + ".00";
	//draw target time
	wstring w_timeString = std::wstring(timeString.begin(), timeString.end());
	m_spriteFont->DrawString(m_spriteBatch.get(), w_timeString.c_str(), float2(30.0f, SCREEN_HEIGHT - 130.0f), color);
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
	m_fruitTextColors[DRAGON] = { 1.0f, 0.3f, 0.3f, 1.f };

	m_spriteBatch = std::make_unique<SpriteBatch>(Renderer::getDeviceContext());
	m_states = std::make_unique<CommonStates>(Renderer::getDevice());

	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	HRESULT t = CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/background.png",
		resource.GetAddressOf(), m_backgroundTexture.ReleaseAndGetAddressOf());
	if (FAILED(t))
		ErrorLogger::logError("Failed to create backgorund sprite texture", t);


	t = CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/stamina.png",
		resource.GetAddressOf(), m_staminaTexture.ReleaseAndGetAddressOf());
	if (FAILED(t))
		ErrorLogger::logError("Failed to create stamina sprite texture", t);


	t = CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/staminaFrame.png",
		resource.GetAddressOf(), m_staminaFrame.ReleaseAndGetAddressOf());
	if (FAILED(t))
		ErrorLogger::logError("Failed to create stamina frame texture", t);

	// create on screen fruit items
	initilizeFruitItem(m_fruitSprites[APPLE], "apple.png", float4(1.f, 0.f, 0.f, 1.f));
	initilizeFruitItem(m_fruitSprites[BANANA], "banana.png", float4(0.9f, 0.7f, 0.2f, 1.f));
	initilizeFruitItem(m_fruitSprites[MELON], "melon.png", float4(0.4f, 0.7f, 0.3f, 1.f));
	initilizeFruitItem(m_fruitSprites[DRAGON], "dragonfruit.png", float4(1.0f, 0.3f, 0.3f, 1.f));
}

HUD::~HUD() {
	m_spriteBatch.reset();
	m_states.reset();
}

void HUD::initilizeFruitItem(OnScreenFruitItem& item, string imageFilename, float4 textColor) {
	
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	wstring fileName = wstring(imageFilename.begin(), imageFilename.end());
	wstring filePath = L"assets/sprites/" + fileName;

	HRESULT t = CreateWICTextureFromFile(Renderer::getDevice(), filePath.c_str(),
		resource.GetAddressOf(), item.texture.ReleaseAndGetAddressOf());
	if (FAILED(t))
		ErrorLogger::logError("Failed to create fruit sprite texture", t);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	resource.As(&tex);
	CD3D11_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

	// Set all sprites to the same size
	item.scale = 75.0f / (float)texDesc.Height;
	// color
	item.textColor = textColor;
}

void HUD::draw() {
	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());
	m_backgroundPos = float2(20.0f, SCREEN_HEIGHT - 130.0f);
	m_staminaPos = float2(SCREEN_WIDTH - 230.0f, SCREEN_HEIGHT - 60.0f);

	// Draw text background
	float stamina = SceneManager::getScene()->m_player->getStamina();
	m_spriteBatch->Draw(m_backgroundTexture.Get(), m_backgroundPos);
	m_spriteBatch->Draw(m_staminaTexture.Get(), m_staminaPos, nullptr, Colors::White, 0.0f,
		float2(0.0f, 0.0f), float2(stamina + 0.05f, 0.8f));
	m_spriteBatch->Draw(m_staminaFrame.Get(),
		float2(m_staminaPos.x - 13.0f, m_staminaPos.y - 10.0f), nullptr, Colors::White, 0.0f,
		float2(0.0f, 0.0f), float2(1.05f, 0.8f));

	m_spriteBatch->End();

	m_spriteBatch->Begin();

	// Draw time and target time
	float time = SceneManager::getScene()->m_timer.getTimePassed();
	size_t rest = int((time - int(time)) * 100);
	string timeString = "   Time: " + Time2DisplayableString(time)+"."+(rest<10?"0":"")+to_string(rest);
	wstring w_timeString = wstring(timeString.begin(), timeString.end());
	m_spriteFont->DrawString(
		m_spriteBatch.get(), w_timeString.c_str(), float2(30.0f, SCREEN_HEIGHT - 80.0f));
	drawTargetTime();

	// Draw inventory numbers and fruit sprites
	int* gathered = SceneManager::getScene()->m_utility.gathered;		  // NR_OF_FRUITS
	int* winCondition = SceneManager::getScene()->m_utility.winCondition; // NR_OF_FRUITS
	float2 itemPosition = float2(25, 25);
	float2 itemOffset = float2(0, 100);
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		if (winCondition[i] > 0) {
			//text
			wstring displayText = to_wstring(gathered[i]) + L"/" + to_wstring(winCondition[i]);
			m_spriteFont->DrawString(m_spriteBatch.get(), displayText.c_str(),
				itemPosition + float2(75.0f, 0.0f), m_fruitSprites[i].textColor);
			//image
			m_spriteBatch->Draw(m_fruitSprites[i].texture.Get(), itemPosition, nullptr,
				Colors::White, 0.f, float2(0.0f, 0.0f),
				m_fruitSprites[i].scale + 0.1f * m_fruitSprites[i].extraScaling);
			//next item preparation
			itemPosition += itemOffset;
		}
	}


	m_spriteBatch->End();

	// Reset depth state
	setDepthStateToNull();
}
