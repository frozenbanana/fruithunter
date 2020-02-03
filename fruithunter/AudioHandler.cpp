#include "AudioHandler.hpp"
#include "ErrorLogger.hpp"


AudioHandler AudioHandler::m_this;

void AudioHandler::initalize() {
	// Needed to be able to load textures and possibly other things.
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		ErrorLogger::messageBox("Failed to run CoInitalize(NULL) in AudioHandler");
		return;
	}

	// Can add flags to parameters
	m_this.m_audioEngine = std::make_unique<DirectX::AudioEngine>();
	m_this.m_ambientMenu =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), L"assets/sounds/harmony.wav");
	m_this.m_ambientPlay = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/EpicBattle.wav");
	m_this.m_ambientMenuSound = m_ambientMenu->CreateInstance();
	m_this.m_ambientPlaySound = m_ambientPlay->CreateInstance();
}

void AudioHandler::pauseAmbient() {
	m_this.m_ambientMenuSound->Pause();
	m_this.m_ambientPlaySound->Pause();
}

void AudioHandler::startMenuAmbient() {
	m_this.pauseAmbient();
	m_this.m_ambientMenuSound->Play(true);
}

void AudioHandler::startPlayAmbient() {
	m_this.pauseAmbient();
	m_this.m_ambientPlaySound->Play(true);
}


void AudioHandler::playOneTime() {
	// m_soundEffect->Play() // Play one time
}

void AudioHandler::logStats() {
	auto stats = m_this.m_audioEngine->GetStatistics();

	wchar_t buff[256] = {};
	swprintf_s(buff,
		L"Playing: %zu / %zu; Instances %zu; Voices %zu / %zu / %zu / %zu; %zu audio bytes; Output "
		L"Channels: %d; Audio Device present: %d",
		stats.playingOneShots, stats.playingInstances, stats.allocatedInstances,
		stats.allocatedVoices, stats.allocatedVoices3d, stats.allocatedVoicesOneShot,
		stats.allocatedVoicesIdle, stats.audioBytes, m_this.m_audioEngine->GetOutputChannels(),
		m_this.m_audioEngine->IsAudioDevicePresent());

	std::wstring wbuff = std::wstring(buff);
	std::string sbuff = std::string(wbuff.begin(), wbuff.end());
	// ErrorLogger::log(sbuff);
}

AudioHandler* AudioHandler::getInstance() { return &m_this; }
