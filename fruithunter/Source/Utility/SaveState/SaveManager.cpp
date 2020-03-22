#include "SaveManager.h"
#include <fstream>
#include "ErrorLogger.h"

SaveManager SaveManager::m_this;

bool SaveManager::loadFile(string filename, LevelData* levels) const {
	fstream file;
	string path = m_prePath + filename;
	file.open(path, ios::in | ios::binary);
	if (file.is_open()) {
		// nr of levels
		size_t count = 0;
		file.read((char*)&count, sizeof(size_t));
		if (count == NR_OF_LEVELS) {
			if (levels == nullptr)
				levels = new LevelData[NR_OF_LEVELS];
			for (size_t i = 0; i < NR_OF_LEVELS; i++) {
				// is completed
				file.read((char*)&levels[i].isCompleted, sizeof(bool));
				// timeofCompleteion
				file.read((char*)&levels[i].timeOfCompletion, sizeof(size_t));
				// grade
				file.read((char*)&levels[i].grade, sizeof(TimeTargets));
			}
		}
		else {
			ErrorLogger::logWarning(
				"(SaveManager) File is corrupted! Nr of levels is incorrect!\nFile: " + path +
				"\nIncorrectNrOfLevels: " + to_string(count));
		}
		file.close();
		return true;
	}
	else {
		return false;
	}
}

bool SaveManager::saveFile(string filename, LevelData *levels) const {
	if (levels != nullptr) {
		fstream file;
		string path = m_prePath + filename;
		file.open(path, ios::out | ios::binary);
		if (file.is_open()) {
			// nr of levels
			size_t count = NR_OF_LEVELS;
			file.write((char*)&count, sizeof(size_t));
			for (size_t i = 0; i < NR_OF_LEVELS; i++) {
				// is completed
				file.write((char*)&levels[i].isCompleted, sizeof(bool));
				// timeofCompleteion
				file.write((char*)&levels[i].timeOfCompletion, sizeof(size_t));
				// grade
				file.write((char*)&levels[i].grade, sizeof(TimeTargets));
			}

			file.close();
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}

void SaveManager::saveLoaded() {
	if (m_loadedSlot != -1) {
		string filename = "save" + to_string(m_loadedSlot);
		saveFile(filename, m_activeState);
	}
}

SaveManager::SaveManager() { load(0); }
SaveManager::~SaveManager() { saveLoaded(); }

SaveManager* SaveManager::getInstance() { return &m_this; }

const LevelData* SaveManager::getActiveSave() const { return m_activeState; }

vector<LevelData*> SaveManager::getAllSaveStates() const { 
	vector<LevelData*> saves;
	saves.resize(MAX_SAVE_SLOTS);
	for (size_t i = 0; i < saves.size(); i++) {
		loadFile("save" + to_string(i), saves[i]);
	}
	return saves;
}

void SaveManager::setLevelCompletion(size_t index, size_t timeOfCompletion, TimeTargets grade) { 
	if (m_activeState[index].isCompleted) {
		//already completed
		if (timeOfCompletion < m_activeState[index].timeOfCompletion) {
			m_activeState[index].timeOfCompletion = timeOfCompletion;
			m_activeState[index].grade = grade;
		}
	}
	else {
		//completed level
		m_activeState[index].isCompleted = true;
		m_activeState[index].timeOfCompletion = timeOfCompletion;
		m_activeState[index].grade = grade;
	}
}

void SaveManager::resetSaveState() {
	for (size_t i = 0; i < NR_OF_LEVELS; i++) {
		m_activeState[i] = LevelData();
	}
}

bool SaveManager::load(size_t slot) {
	saveLoaded();

	string filename = "save" + to_string(slot);
	m_loadedSlot = slot;
	return loadFile(filename, m_activeState);
}
