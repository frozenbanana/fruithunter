#include "AudioHandler.h"
#include "ErrorLogger.h"
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

void AudioHandler::pauseInstance(AudioHandler::Sounds sound) {
	m_soundEffectsInstance[sound]->Stop(true); // Play one time
}

void AudioHandler::playOnce(AudioHandler::Sounds sound) {
	m_soundEffects[sound]->Play(); // Play one time
}

void AudioHandler::playInstance(AudioHandler::Sounds sound) {
	if (m_soundEffectsInstance[sound]->GetState() != SoundState::PLAYING) {
		ErrorLogger::log("I am actually starting");
		m_soundEffectsInstance[sound]->Play();
	}
}
void AudioHandler::playInstance(AudioHandler::Sounds sound, float coefficient) {
	if (m_soundEffectsInstance[sound]->GetState() != SoundState::PLAYING && coefficient < 0.99f) {
		m_soundEffectsInstance[sound]->Play();
	}
}

void AudioHandler::playOnceByDistance(
	AudioHandler::Sounds sound, float3 listnerPosition, float3 soundPosition) {
	float distance = (listnerPosition - soundPosition).Length();
	float volume = 1.f - map(0.f, m_maxHearingDistance, 0.f, 1.f, distance);
	// Tweak to volume change more realistic
	volume *= volume;
	ErrorLogger::log("volume: " + to_string(volume));
	m_soundEffects[sound]->Play(volume, 0.f, 0.f);
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


	// convert from wide char to narrow char array
	char ch[256];
	char DefChar = ' ';
	WideCharToMultiByte(CP_ACP, 0, buff, -1, ch, 256, &DefChar, NULL); // No error checking

	std::string sbuff = std::string(ch);
	ErrorLogger::log(sbuff);
}

AudioHandler* AudioHandler::getInstance() { return &m_this; }
