#include "AudioHandler.h"
#include "ErrorLogger.h"
#include "Settings.h"
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
	m_this.m_soundEffects[SLOW_MOTION] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/slowmotion.wav");
	m_this.m_soundEffects[DING_1] =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), L"assets/sounds/ding1.wav");
	m_this.m_soundEffects[DING_2] =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), L"assets/sounds/ding2.wav");
	m_this.m_soundEffects[DING_3] =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), L"assets/sounds/ding3.wav");
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
	m_this.m_soundEffects[BEAR_EATING] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/bear-eating.wav");
	m_this.m_soundEffects[BEAR_HAPPY] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/bear-happy.wav");
	m_this.m_soundEffects[BEAR_PUSH] =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), L"assets/sounds/bear-push.wav");
	m_this.m_soundEffects[GOAT_EATING] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/goat-eating.wav");
	m_this.m_soundEffects[GOAT_HAPPY] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/goat-happy.wav");
	m_this.m_soundEffects[GOAT_PUSH] =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), L"assets/sounds/goat-push.wav");
	m_this.m_soundEffects[GORILLA_EATING] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/gorilla-eating.wav");
	m_this.m_soundEffects[GORILLA_HAPPY] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/gorilla-happy.wav");
	m_this.m_soundEffects[GORILLA_PUSH] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/gorilla-push.wav");
	m_this.m_soundEffects[SLEEPING] =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), L"assets/sounds/snooring.wav");
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
	m_this.m_music[ELEVATOR] = std::make_unique<DirectX::SoundEffect>(
		m_audioEngine.get(), L"assets/sounds/elevator-music.wav");

	m_this.m_musicInstances[JINGLE_GUITAR] = m_this.m_music[JINGLE_GUITAR]->CreateInstance();
	m_this.m_musicInstances[SPANISH_GUITAR] = m_this.m_music[SPANISH_GUITAR]->CreateInstance();
	m_this.m_musicInstances[KETAPOP] = m_this.m_music[KETAPOP]->CreateInstance();
	m_this.m_musicInstances[KETAPOP_DARK] = m_this.m_music[KETAPOP_DARK]->CreateInstance();
	m_this.m_musicInstances[ELEVATOR] = m_this.m_music[ELEVATOR]->CreateInstance();
	m_this.m_musicInstances[JINGLE_GUITAR]->SetVolume(m_musicVolume);
	m_this.m_musicInstances[SPANISH_GUITAR]->SetVolume(m_musicVolume);
	m_this.m_musicInstances[KETAPOP]->SetVolume(m_musicVolume);
	m_this.m_musicInstances[KETAPOP_DARK]->SetVolume(m_musicVolume);
	m_this.m_musicInstances[ELEVATOR]->SetVolume(m_musicVolume);

	m_oldMusic = Music::MUSIC_LENGTH;
}

void AudioHandler::playMusic(AudioHandler::Music music) {
	m_music[music]->Play(m_musicVolume * m_masterVolume, 0.f, 0.f);
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
		coefficient = max(m_timer.getTimePassed() / timeLimit, m_musicVolume);
		m_this.m_musicInstances[m_currentMusic]->SetVolume(m_musicVolume - coefficient);
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
		m_this.m_musicInstances[newMusic]->SetVolume(m_musicVolume * m_masterVolume);
		m_this.m_musicInstances[newMusic]->Play(true);
		m_currentMusic = newMusic;
	}
}

void AudioHandler::changeMusicByTag(AreaTags tag, float dt) {
	switch (tag) {
	case AreaTags::Forest:
		changeMusicTo(AudioHandler::Music::KETAPOP, dt);
		break;
	case AreaTags::Desert:
		changeMusicTo(AudioHandler::Music::SPANISH_GUITAR, dt);
		break;
	case AreaTags::Plains:
		changeMusicTo(AudioHandler::Music::JINGLE_GUITAR, dt);
		break;
	case AreaTags::Volcano:
		changeMusicTo(AudioHandler::Music::KETAPOP_DARK, dt);
		break;
	default:
		ErrorLogger::log("No known terrain tag");
		changeMusicTo(AudioHandler::Music::ELEVATOR, dt);
	}
}

void AudioHandler::pauseInstance(AudioHandler::Sounds sound) {
	m_this.m_soundEffectsInstance[sound]->Stop(true); // Play one time
}

void AudioHandler::playOnce(AudioHandler::Sounds sound) {
	m_this.m_soundEffects[sound]->Play(m_effectsVolume * m_masterVolume, 0.f, 0.f); // Play one time
}

void AudioHandler::playInstance(AudioHandler::Sounds sound) {
	if (m_this.m_soundEffectsInstance[sound]->GetState() != SoundState::PLAYING) {
		m_this.m_soundEffectsInstance[sound]->SetVolume(m_effectsVolume * m_masterVolume);
		m_this.m_soundEffectsInstance[sound]->Play();
	}
}
void AudioHandler::playInstance(AudioHandler::Sounds sound, float coefficient) {
	if (m_this.m_soundEffectsInstance[sound]->GetState() != SoundState::PLAYING &&
		coefficient < 0.99f) {
		m_this.m_soundEffectsInstance[sound]->SetVolume(m_effectsVolume * m_masterVolume);
		m_this.m_soundEffectsInstance[sound]->Play();
	}
}

void AudioHandler::playOnceByDistance(
	AudioHandler::Sounds sound, float3 listnerPosition, float3 soundPosition) {
	float distance = (listnerPosition - soundPosition).Length();
	float volume = 1.f - map(0.f, m_maxHearingDistance, 0.f, 1.f, distance);
	// Tweak to volume change more realistic
	volume *= volume;
	m_this.m_soundEffects[sound]->Play(volume * m_effectsVolume * m_masterVolume, 0.f, 0.f);
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

void AudioHandler::setMasterVolume(float value) {
	m_masterVolume = value;
	m_this.m_musicInstances[m_currentMusic]->SetVolume(m_musicVolume * m_masterVolume);
}

void AudioHandler::setMusicVolume(float value) {
	m_musicVolume = value;
	m_this.m_musicInstances[m_currentMusic]->SetVolume(m_musicVolume * m_masterVolume);
}

void AudioHandler::setEffectsVolume(float value) { m_effectsVolume = value; }
