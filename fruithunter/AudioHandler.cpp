#include "AudioHandler.hpp"
#include "ErrorLogger.hpp"

AudioHandler::AudioHandler() {
	// Can add flags to parameters
	m_audioEngine = std::make_unique<DirectX::AudioEngine>();
	m_soundEffect =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), L"assets/sounds/goat.wav");
}

void AudioHandler::play() {
	auto goatSound = m_soundEffect->CreateInstance();
	goatSound->Play();
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
	ErrorLogger::log(sbuff);
}