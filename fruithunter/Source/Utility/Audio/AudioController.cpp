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
		std::make_unique<DirectX::SoundEffect>(&m_audioEngine, wstr.c_str());
	m_library[m_library.size() - 1].name = sound;
	if (m_library[m_library.size() - 1].effect.get() == nullptr) {
		//failed loading
		m_library.pop_back();
		return false;
	}
	return true;
}

SoundID AudioController::genID() { return ++m_idBank; }

AudioController::AudioController() {

	// Needed to be able to load textures and possibly other things.
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		ErrorLogger::messageBox("Failed to run CoInitalize(NULL) in AudioHandler");
		return;
	}

}

AudioController* AudioController::getInstance() { return &m_this; }

void AudioController::setMasterVolume(float value) { m_masterVolume = value; }

void AudioController::setMusicVolume(float value) { m_musicVolume = value; }

void AudioController::setEffectsVolume(float value) { m_effectsVolume = value; }

void AudioController::update() {
	for (size_t i = 0; i < m_playlist.size(); i++) {
		if (m_playlist[i].instance->GetState() == SoundState::PAUSED ||
			m_playlist[i].instance->GetState() == SoundState::STOPPED) {
			m_playlist.erase(m_playlist.begin() + i);
			i--;
		}
	}
}

SoundID AudioController::play(string sound, bool repeat) {
	int effectIndex = findEffect(sound);
	if (effectIndex == -1) {
		// not found
		if (load(sound) == false) {
			// failed to load
			ErrorLogger::logError("(AudioController) failed loading sound: " + sound);
			return NULL;
		}
		effectIndex = m_library.size() - 1;
	}
	// create instance
	m_playlist.push_back(SoundInstance());
	m_playlist.back().instance = m_library[effectIndex].effect->CreateInstance();
	SoundID id = genID();
	m_playlist.back().id = id;
	//start instance
	m_playlist.back().instance->SetVolume(m_masterVolume);
	m_playlist.back().instance->Play(repeat);

	return id;
}

void AudioController::stop(SoundID id) {
	int index = findID(id);
	if (index != -1) {
		m_playlist[index].instance->Stop();
		m_playlist.erase(m_playlist.begin()+index);
	}
}

bool AudioController::isListed(SoundID id) { return (findID(id) != -1); }
