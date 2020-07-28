#include "AudioHandler.h"
#include "ErrorLogger.h"
#include "Settings.h"

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
	AudioHandler* ah = AudioHandler::getInstance();
	// Needed to be able to load textures and possibly other things.
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		ErrorLogger::messageBox("Failed to run CoInitalize(NULL) in AudioHandler");
		return;
	}

	// Can add flags to parameters
	ah->m_audioEngine = std::make_unique<DirectX::AudioEngine>();
	// One time sound effects
	string files_effect_wav[Sounds::SOUNDS_LENGTH] = { "assets/sounds/applause.wav",
		"assets/sounds/slowmotion.wav", "assets/sounds/slowmotion-reversed.wav",
		"assets/sounds/ding1.wav", "assets/sounds/ding2.wav", "assets/sounds/ding3.wav",
		"assets/sounds/light-arrow-release.wav", "assets/sounds/heavy-arrow-release.wav",
		"assets/sounds/stretch-bow.wav", "assets/sounds/hit-wood.wav",
		"assets/sounds/fruit-impact-wet.wav", "assets/sounds/collected-item.wav",
		"assets/sounds/bear-push.wav", "assets/sounds/bear-happy.wav",
		"assets/sounds/bear-eating.wav", "assets/sounds/goat-push.wav",
		"assets/sounds/goat-happy.wav", "assets/sounds/goat-eating.wav",
		"assets/sounds/gorilla-push.wav", "assets/sounds/gorilla-happy.wav",
		"assets/sounds/gorilla-eating.wav", "assets/sounds/snooring.wav" };
	for (size_t i = 0; i < Sounds::SOUNDS_LENGTH; i++) {
		std::wstring widestr = std::wstring(files_effect_wav[i].begin(), files_effect_wav[i].end());
		const wchar_t* widecstr = widestr.c_str();
		ah->m_soundEffects[i] =
			std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), widecstr);
	}
	// Some effect require instances for more control
	ah->m_soundEffectsInstance[STRETCH_BOW] =
		ah->m_soundEffects[STRETCH_BOW]->CreateInstance();

	// Ambient sounds
	string wavFiles[Music::MUSIC_LENGTH] = { "assets/sounds/jingle-guitar.wav ",
		"assets/sounds/spanish-guitar.wav", "assets/sounds/ketapop-nudia-short.wav",
		"assets/sounds/ketapop-dark-short.wav", "assets/sounds/elevator-music.wav",
		"assets/sounds/ocean-music.wav" };
	for (size_t i = 0; i < Music::MUSIC_LENGTH; i++) {
		std::wstring widestr = std::wstring(wavFiles[i].begin(), wavFiles[i].end());
		const wchar_t* widecstr = widestr.c_str();
		ah->m_music[i] = std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), widecstr);
		ah->m_musicInstances[i] = ah->m_music[i]->CreateInstance();
		ah->m_musicInstances[i]->SetVolume(m_musicVolume);
	}

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
	AudioHandler* ah = AudioHandler::getInstance();
	ah->m_musicInstances[newMusic]->Play(true);
	m_timer.reset();
	float coefficient;
	float timeLimit = 3.0f;
	while (m_timer.getTimePassed() < timeLimit) {
		m_timer.update();
		coefficient = max(m_timer.getTimePassed() / timeLimit, m_musicVolume);
		ah->m_musicInstances[m_currentMusic]->SetVolume(m_musicVolume - coefficient);
		ah->m_musicInstances[newMusic]->SetVolume(coefficient);
	}
	ah->m_musicInstances[m_currentMusic]->Pause();
	m_currentMusic = newMusic;
}

void AudioHandler::changeMusicTo(AudioHandler::Music newMusic, float dt) {
	AudioHandler* ah = AudioHandler::getInstance();
	if (newMusic != m_currentMusic) {
		// Simple transition
		ah->m_musicInstances[m_currentMusic]->Pause();
		ah->m_musicInstances[newMusic]->SetVolume(m_musicVolume * m_masterVolume);
		ah->m_musicInstances[newMusic]->Play(true);
		m_currentMusic = newMusic;
	}
}

void AudioHandler::changeMusicByTag(AreaTag tag, float dt) {
	static Music mapping[AreaTag::NR_OF_AREAS] = { Music::KETAPOP, Music::JINGLE_GUITAR,
		Music::SPANISH_GUITAR, Music::KETAPOP_DARK, Music::ELEVATOR };
	changeMusicTo(mapping[tag], dt);
}

void AudioHandler::pauseInstance(AudioHandler::Sounds sound) {
	AudioHandler* ah = AudioHandler::getInstance();
	ah->m_soundEffectsInstance[sound]->Stop(true); // Play one time
}

void AudioHandler::playOnce(AudioHandler::Sounds sound) {
	AudioHandler* ah = AudioHandler::getInstance();
	ah->m_soundEffects[sound]->Play(m_effectsVolume * m_masterVolume, 0.f, 0.f); // Play one time
}

void AudioHandler::playInstance(AudioHandler::Sounds sound) {
	AudioHandler* ah = AudioHandler::getInstance();
	if (ah->m_soundEffectsInstance[sound]->GetState() != SoundState::PLAYING) {
		ah->m_soundEffectsInstance[sound]->SetVolume(m_effectsVolume * m_masterVolume);
		ah->m_soundEffectsInstance[sound]->Play();
	}
}
void AudioHandler::playInstance(AudioHandler::Sounds sound, float coefficient) {
	AudioHandler* ah = AudioHandler::getInstance();
	if (ah->m_soundEffectsInstance[sound]->GetState() != SoundState::PLAYING &&
		coefficient < 0.99f) {
		ah->m_soundEffectsInstance[sound]->SetVolume(m_effectsVolume * m_masterVolume);
		ah->m_soundEffectsInstance[sound]->Play();
	}
}

void AudioHandler::playOnceByDistance(
	AudioHandler::Sounds sound, float3 listnerPosition, float3 soundPosition) {
	float distance = (listnerPosition - soundPosition).Length();
	float volume = 1.f - map(0.f, m_maxHearingDistance, 0.f, 1.f, distance);
	// Tweak to volume change more realistic
	volume *= volume;
	AudioHandler::getInstance()->m_soundEffects[sound]->Play(
		volume * m_effectsVolume * m_masterVolume, 0.f, 0.f);
}

void AudioHandler::logStats() {
	AudioHandler* ah = AudioHandler::getInstance();
	auto stats = ah->m_audioEngine->GetStatistics();

	wchar_t buff[256] = {};
	swprintf_s(buff,
		L"Playing: %zu / %zu; Instances %zu; Voices %zu / %zu / %zu / %zu; %zu audio bytes; "
		L"Output "
		L"Channels: %d; Audio Device present: %d",
		stats.playingOneShots, stats.playingInstances, stats.allocatedInstances,
		stats.allocatedVoices, stats.allocatedVoices3d, stats.allocatedVoicesOneShot,
		stats.allocatedVoicesIdle, stats.audioBytes, ah->m_audioEngine->GetOutputChannels(),
		ah->m_audioEngine->IsAudioDevicePresent());


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
	AudioHandler::getInstance()->m_musicInstances[m_currentMusic]->SetVolume(
		m_musicVolume * m_masterVolume);
}

void AudioHandler::setMusicVolume(float value) {
	m_musicVolume = value;
	AudioHandler::getInstance()->m_musicInstances[m_currentMusic]->SetVolume(
		m_musicVolume * m_masterVolume);
}

void AudioHandler::setEffectsVolume(float value) { m_effectsVolume = value; }
