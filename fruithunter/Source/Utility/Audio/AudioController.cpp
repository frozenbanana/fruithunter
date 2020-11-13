#include "AudioController.h"
#include "ErrorLogger.h"

AudioController AudioController::m_this;

int AudioController::findID(SoundID id) { 
	for (size_t i = 0; i < m_playlist.size(); i++) {
		if (m_playlist[i].id == id)
			return i;
	}
	return -1;
}

int AudioController::findEffect(string sound) {
	for (size_t i = 0; i < m_library.size(); i++) {
		if (m_library[i].name == sound)
			return i;
	}
	return -1;
}

bool AudioController::load(string sound) {
	m_library.resize(m_library.size() + 1);
	string path = m_pre_path + sound + m_fileEnding;
	wstring wstr = wstring(path.begin(), path.end());
	m_library[m_library.size() - 1].effect =
		std::make_unique<DirectX::SoundEffect>(m_audioEngine.get(), wstr.c_str());
	m_library[m_library.size() - 1].name = sound;
	if (m_library[m_library.size() - 1].effect.get() == nullptr) {
		//failed loading
		m_library.pop_back();
		ErrorLogger::logError("(AudioController) failed loading sound: " + path);
		return false;
	}
	return true;
}

float AudioController::getVolume(SoundType type) const {
	if (type == SoundType::Effect)
		return m_masterVolume * m_effectsVolume;
	else if (type == SoundType::Music)
		return m_masterVolume * m_musicVolume;
	else
		return m_masterVolume;
}

void AudioController::updatePlaylistVolumes() {
	for (size_t i = 0; i < m_playlist.size(); i++) {
		m_playlist[i].instance->SetVolume(getVolume(m_playlist[i].type));
	}
}

void AudioController::updateInstanceVolume(SoundInstance* instance) {
	if (instance) {
		instance->instance->SetVolume(getVolume(instance->type)*instance->volume);
	}
}

SoundID AudioController::genID() { return ++m_idBank; }

AudioController::AudioController() {

	// Needed to be able to load textures and possibly other things.
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		ErrorLogger::messageBox("Failed to run CoInitalize(NULL) in AudioHandler");
		return;
	}

	/*AUDIO_ENGINE_FLAGS eflags = AudioEngine_EnvironmentalReverb | AudioEngine_ReverbUseFilters;
	m_audioEngine = std::make_unique<AudioEngine>(eflags);
	m_audioEngine->SetReverb(Reverb_ConcertHall);*/

	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
	m_audioEngine = std::make_unique<AudioEngine>(eflags);
}

AudioController* AudioController::getInstance() { return &m_this; }

void AudioController::setMasterVolume(float value) { 
	m_masterVolume = value; 
	updatePlaylistVolumes();
}

void AudioController::setMusicVolume(float value) {
	m_musicVolume = value;
	updatePlaylistVolumes();
}

void AudioController::setEffectsVolume(float value) {
	m_effectsVolume = value;
	updatePlaylistVolumes();
}

void AudioController::update() {
	m_timer.update();
	float dt = m_timer.getDt();

	m_audioEngine->Update();

	// Remove paused and stopped instances
	for (size_t i = 0; i < m_playlist.size(); i++) {
		if (m_playlist[i].instance->GetState() == SoundState::PAUSED ||
			m_playlist[i].instance->GetState() == SoundState::STOPPED) {
			m_playlist.erase(m_playlist.begin() + i);
			i--;
		}
	}

	// Handle events
	for (size_t i = 0; i < m_events.size(); i++) {
		if (m_events[i]->update(dt)) {
			m_events.erase(m_events.begin() + i);
			i--;
		}
	}
}

void AudioController::flush() { m_playlist.clear(); }

SoundID AudioController::play(string sound, AudioController::SoundType type, bool repeat) {
	int effectIndex = findEffect(sound);
	if (effectIndex == -1) {
		// not found in library
		if (load(sound) == false) {
			return NULL;// failed
		}
		effectIndex = m_library.size() - 1;
	}
	// create instance
	m_playlist.push_back(SoundInstance());
	m_playlist.back().instance = m_library[effectIndex].effect->CreateInstance(); 
	SoundID id = genID();
	m_playlist.back().id = id;
	m_playlist.back().type = type;
	//start instance
	m_playlist.back().instance->SetVolume(getVolume(type));
	m_playlist.back().instance->Play(repeat);

	return id;
}

//SoundID AudioController::play3D(string sound, AudioController::SoundType type, bool repeat) {
//	int effectIndex = findEffect(sound);
//	if (effectIndex == -1) {
//		// not found
//		if (load(sound) == false) {
//			// failed to load
//			ErrorLogger::logError("(AudioController) failed loading sound: " + sound);
//			return NULL;
//		}
//		effectIndex = m_library.size() - 1;
//	}
//
//	// create instance
//	m_playlist.push_back(SoundInstance());
//	m_playlist.back().instance = m_library[effectIndex].effect->CreateInstance(
//		SoundEffectInstance_Use3D | SoundEffectInstance_ReverbUseFilters);
//	SoundID id = genID();
//	m_playlist.back().id = id;
//	m_playlist.back().type = type;
//	m_playlist.back().use3D = true;
//	// start instance
//	m_playlist.back().instance->SetVolume(m_masterVolume);
//	m_playlist.back().instance->Play(repeat);
//
//	return id;
//}

void AudioController::scaleVolumeByDistance(SoundID id, float distance, float min, float max) {
	int index = findID(id);
	if (index != -1) {
		float scale = 1-Clamp<float>((distance - min) / (max - min), 0, 1);
		m_playlist[index].volume = pow(scale, 2);
		m_playlist[index].instance->SetVolume(
			getVolume(m_playlist[index].type) * m_playlist[index].volume);
	}
}

void AudioController::setVolume(SoundID id, float value) {
	int index = findID(id);
	if (index != -1) {
		m_playlist[index].volume = value;
	}
}

void AudioController::fadeOut(SoundID id, float time) { 
	m_events.push_back(make_shared<EventFadeOut>(id, time));
}

void AudioController::fadeIn(SoundID id, float time) {
	m_events.push_back(make_shared<EventFadeIn>(id, time));
}

void AudioController::stop(SoundID id) {
	int index = findID(id);
	if (index != -1) {
		m_playlist[index].instance->Stop();
		m_playlist.erase(m_playlist.begin()+index);
	}
}

bool AudioController::isListed(SoundID id) { return (findID(id) != -1); }
