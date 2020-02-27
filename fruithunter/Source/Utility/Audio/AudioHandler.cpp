#include "AudioHandler.h"
#include "ErrorLogger.h"
//#include <thread> // std::thread
AudioHandler AudioHandler::m_this;


float map(float low, float high, float newLow, float newHigh, float value) {
	float oldCoefficient = (value / (low + (high - low)));
	float newRange = (newHigh - newLow) + newLow;
	return oldCoefficient * newRange;
}

bool AudioHandler::isPlaying(AudioHandler::Sounds sound) {
	return m_soundEffects[sound]->IsInUse();
}

void AudioHandler::initalize() {
	// Needed to be able to load textures and possibly other things.
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		ErrorLogger::messageBox("Failed to run CoInitalize(NULL) in AudioHandler");
		return;
	}

	// Can add flags to parameters
	m_this.m_audioEngine = std::make_unique<DirectX::AudioEngine>();
	// One time sound effects
	m_this.m_soundEffects[LIGHT_ARROW] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/light-arrow-release.wav");
	m_this.m_soundEffects[HEAVY_ARROW] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/heavy-arrow-release.wav");
	m_this.m_soundEffects[STRETCH_BOW] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/stretch-bow.wav");
	m_this.m_soundEffects[HIT_WOOD] =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), L"assets/sounds/hit-wood.wav");
	m_this.m_soundEffects[HIT_FRUIT] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/fruit-impact-wet.wav");
	m_this.m_soundEffects[COLLECT] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/collected-item.wav");

	// Some effect require instances for more control
	m_this.m_soundEffectsInstance[STRETCH_BOW] =
		m_this.m_soundEffects[STRETCH_BOW]->CreateInstance();

	// Ambient sounds
	m_this.m_music[JINGLE_GUITAR] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/jingle-guitar.wav");

	m_this.m_music[SPANISH_GUITAR] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/spanish-guitar.wav");
	m_this.m_music[KETAPOP] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/ketapop-nudia-short.wav");
	m_this.m_music[KETAPOP_DARK] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/ketapop-dark-short.wav");

	m_this.m_musicInstances[JINGLE_GUITAR] = m_this.m_music[JINGLE_GUITAR]->CreateInstance();
	m_this.m_musicInstances[SPANISH_GUITAR] = m_this.m_music[SPANISH_GUITAR]->CreateInstance();
	m_this.m_musicInstances[KETAPOP] = m_this.m_music[KETAPOP]->CreateInstance();
	m_this.m_musicInstances[KETAPOP_DARK] = m_this.m_music[KETAPOP_DARK]->CreateInstance();

	m_oldMusic = Music::MUSIC_LENGTH;
}

void AudioHandler::playMusic(AudioHandler::Music music) {
	m_music[music]->Play();
	m_currentMusic = music;
}

void AudioHandler::pauseAllMusic() {
	for (size_t i = 0; i < MUSIC_LENGTH; i++) {
		m_musicInstances[i]->Pause();
	}
}

// This is causing the game to freeze, should make music transition smooth.
void AudioHandler::doTransition(AudioHandler::Music newMusic) {
	m_this.m_musicInstances[newMusic]->Play(true);
	m_timer.reset();
	float coefficient;
	float timeLimit = 3.0f;
	while (m_timer.getTimePassed() < timeLimit) {
		m_timer.update();
		coefficient = m_timer.getTimePassed() / timeLimit;
		m_this.m_musicInstances[m_currentMusic]->SetVolume(1.f - coefficient);
		m_this.m_musicInstances[newMusic]->SetVolume(coefficient);
	}
	m_this.m_musicInstances[m_currentMusic]->Pause();
	m_currentMusic = newMusic;
}

void AudioHandler::changeMusicTo(AudioHandler::Music newMusic, float dt) {
	if (newMusic != m_currentMusic) {
		/*thread td([this, newMusic] { this->AudioHandler::doTransition(newMusic); });
		td.detach();*/

		// Simple transition
		m_this.m_musicInstances[m_currentMusic]->Pause();
		m_this.m_musicInstances[newMusic]->Play(true);
		m_currentMusic = newMusic;
	}
}

void AudioHandler::pauseInstance(AudioHandler::Sounds sound) {
	m_this.m_soundEffectsInstance[sound]->Stop(true); // Play one time
}

void AudioHandler::playOnce(AudioHandler::Sounds sound) {
	m_this.m_soundEffects[sound]->Play(); // Play one time
}

void AudioHandler::playInstance(AudioHandler::Sounds sound) {
	if (m_this.m_soundEffectsInstance[sound]->GetState() != SoundState::PLAYING) {
		m_this.m_soundEffectsInstance[sound]->Play();
	}
}
void AudioHandler::playInstance(AudioHandler::Sounds sound, float coefficient) {
	if (m_this.m_soundEffectsInstance[sound]->GetState() != SoundState::PLAYING &&
		coefficient < 0.99f) {
		m_this.m_soundEffectsInstance[sound]->Play();
	}
}

void AudioHandler::playOnceByDistance(
	AudioHandler::Sounds sound, float3 listnerPosition, float3 soundPosition) {
	float distance = (listnerPosition - soundPosition).Length();
	float volume = 1.f - map(0.f, m_maxHearingDistance, 0.f, 1.f, distance);
	// Tweak to volume change more realistic
	volume *= volume;
	m_this.m_soundEffects[sound]->Play(volume, 0.f, 0.f);
}

void AudioHandler::logStats() {
	auto stats = m_this.m_audioEngine->GetStatistics();

	wchar_t buff[256] = {};
	swprintf_s(buff,
		L"Playing: %zu / %zu; Instances %zu; Voices %zu / %zu / %zu / %zu; %zu audio bytes; "
		L"Output "
		L"Channels: %d; Audio Device present: %d",
		stats.playingOneShots, stats.playingInstances, stats.allocatedInstances,
		stats.allocatedVoices, stats.allocatedVoices3d, stats.allocatedVoicesOneShot,
		stats.allocatedVoicesIdle, stats.audioBytes, m_this.m_audioEngine->GetOutputChannels(),
		m_this.m_audioEngine->IsAudioDevicePresent());


	// convert from wide char to narrow char array
	char ch[256];
	char DefChar = ' ';
	WideCharToMultiByte(CP_ACP, 0, buff, -1, ch, 256, &DefChar, NULL); // No error checking

	std::string sbuff = std::string(ch);
	ErrorLogger::log(sbuff);
}

AudioHandler* AudioHandler::getInstance() { return &m_this; }
