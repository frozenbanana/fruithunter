#include "SaveManager.h"
#include "ErrorLogger.h"
#include <fstream>

SaveManager SaveManager::m_this;

int SaveManager::readFileVersion(ifstream& file) {
	if (file.is_open()) {
		return fileRead<int>(file);
	}
	return 0;
}

void SaveManager::writeFileVersion(
	ofstream& file, int _version) {
	if (file.is_open()) {
		fileWrite<int>(file, _version);
	}
}

void SaveManager::load() {
	ifstream file;
	string path = "";
	path += PLAYER_FILEPATH;
	path += PLAYER_FILENAME;
	file.open(path, ios::binary);
	if (file.is_open()) {
		int version = readFileVersion(file);
		m_progress.read(file, version);
		file.close();
	}
	else {
		//no progress has been saved
		//do nothing
	}
}

void SaveManager::save() {
	ofstream file;
	string path = "";
	path += PLAYER_FILEPATH;
	path += PLAYER_FILENAME;
	file.open(path, ios::binary);
	if (file.is_open()) {
		writeFileVersion(file, m_progress.getVersion());
		m_progress.write(file);
		file.close();
	}
	else {
		cout << "(SaveManager) Failed saving player progress" << endl;
	}
}

SaveManager::SaveManager() { load(); }
SaveManager::~SaveManager() {}

SaveManager* SaveManager::getInstance() { return &m_this; }

bool SaveManager::getLevelProgress(string scene, time_t& timeMs) { 
	for (size_t i = 0; i < m_progress.level_progress.size(); i++) {
		if (m_progress.level_progress[i].scene == scene) {
			timeMs = m_progress.level_progress[i].timeToComplete;
			return true;
		}
	}
	return false;
}

void SaveManager::setLevelProgress(string scene, size_t timeMs) {
	if (scene != "") {
		bool update = false;
		bool found = false;
		for (size_t i = 0; i < m_progress.level_progress.size(); i++) {
			if (m_progress.level_progress[i].scene == scene) {
				// found
				found = true;
				if (timeMs < m_progress.level_progress[i].timeToComplete) {
					// overwrite
					m_progress.level_progress[i].timeToComplete = timeMs;
					update = true;
				}
				break;
			}
		}
		if (!found) {
			ProgressStructs::PlayerProgression_1_0::SceneCompletion level;
			level.scene = scene;
			level.timeToComplete = timeMs;
			m_progress.level_progress.push_back(level);
			update = true;
		}
		if (update)
			save();
	}
}

void ProgressStructs::PlayerProgression_1_0::_read(ifstream& file) {
	if (file.is_open()) {
		size_t length = fileRead<size_t>(file);
		level_progress.resize(length);
		for (size_t i = 0; i < level_progress.size(); i++) {
			// scene
			fileRead(file, level_progress[i].scene);
			// time to complete
			fileRead<size_t>(file, level_progress[i].timeToComplete);
		}
	}
}

void ProgressStructs::PlayerProgression_1_0::_write(ofstream& file) {
	if (file.is_open()) {
		fileWrite<size_t>(file, level_progress.size());
		for (size_t i = 0; i < level_progress.size(); i++) {
			// scene
			fileWrite(file, level_progress[i].scene);
			// time to complete
			fileWrite<size_t>(file, level_progress[i].timeToComplete);
		}
	}
}

void ProgressStructs::PlayerProgression_1_0::_pass(ifstream& file, int _version) {
	// read from old struct (further reads from older if necessary)
	PlayerProgression_Test older;
	older.read(file, _version);
	// parse from old struct
	level_progress.resize(older.level_progress.size());
	for (size_t i = 0; i < older.level_progress.size(); i++) {
		level_progress[i].scene = older.level_progress[i].scene;
		level_progress[i].timeToComplete = older.level_progress[i].time;
	}
}

void ProgressStructs::PlayerProgression_1_0::clear() { level_progress.clear(); }

int ProgressStructs::PlayerProgressionBase::getVersion() const { return version; }

void ProgressStructs::PlayerProgressionBase::read(ifstream& file, int _version) {
	if (version < _version) {
		// OLD SYSTEM
		ErrorLogger::logError("(SaveManager) System to old to read player progress file! (Version "
							  "number is greater than system)");
	}
	else if (version == _version) {
		// current version
		clear();
		_read(file);
	}
	else {
		// older version
		clear();
		_pass(file, _version);
	}
}

void ProgressStructs::PlayerProgressionBase::write(ofstream& file) { _write(file); }

void ProgressStructs::PlayerProgression_Test::clear() { level_progress.clear(); }

void ProgressStructs::PlayerProgression_Test::_read(ifstream& file) {
	if (file.is_open()) {
		size_t length = fileRead<size_t>(file);
		level_progress.resize(length);
		for (size_t i = 0; i < level_progress.size(); i++) {
			// scene
			fileRead(file, level_progress[i].scene);
			// time to complete
			fileRead<size_t>(file, level_progress[i].time);
		}
	}
}

void ProgressStructs::PlayerProgression_Test::_write(ofstream& file) {
	if (file.is_open()) {
		fileWrite<size_t>(file, level_progress.size());
		for (size_t i = 0; i < level_progress.size(); i++) {
			// scene
			fileWrite(file, level_progress[i].scene);
			// time to complete
			fileWrite<size_t>(file, level_progress[i].time);
		}
	}
}

void ProgressStructs::PlayerProgression_Test::_pass(ifstream& file, int _version) {
	ErrorLogger::logError("(SaveManager) Failed reading save file. Player save file version is too old!");
}
