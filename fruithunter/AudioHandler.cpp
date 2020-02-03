#include "AudioHandler.hpp"
#include "ErrorLogger.hpp"

AudioHandler::AudioHandler() {
	// Can add flags to parameters
	m_audioEngine = std::make_unique<DirectX::AudioEngine>();
	m_ambientMenu =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), L"assets/sounds/harmony.wav");
	m_ambientPlay = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/EpicBattle.wav");
	m_ambientMenuSound = m_ambientMenu->CreateInstance();
	m_ambientPlaySound = m_ambientPlay->CreateInstance();
}

void AudioHandler::pauseAmbient() {
	m_ambientMenuSound->Pause();
	m_ambientPlaySound->Pause();
}

void AudioHandler::startMenuAmbient() { m_ambientMenuSound->Play(true); }

void AudioHandler::startPlayAmbient() { m_ambientPlaySound->Play(true); }


void AudioHandler::play() {
	m_audioEngine->Resume();
	// m_soundEffect->Play
}

void AudioHandler::logStats() {
	auto stats = m_audioEngine->GetStatistics();

	wchar_t buff[256] = {};
	swprintf_s(buff,
		L"Playing: %zu / %zu; Instances %zu; Voices %zu / %zu / %zu / %zu; %zu audio bytes; Output "
		L"Channels: %d; Audio Device present: %d",
		stats.playingOneShots, stats.playingInstances, stats.allocatedInstances,
		stats.allocatedVoices, stats.allocatedVoices3d, stats.allocatedVoicesOneShot,
		stats.allocatedVoicesIdle, stats.audioBytes, m_audioEngine->GetOutputChannels(),
		m_audioEngine->IsAudioDevicePresent());

	std::wstring wbuff = std::wstring(buff);
	std::string sbuff = std::string(wbuff.begin(), wbuff.end());
	// ErrorLogger::log(sbuff);
}