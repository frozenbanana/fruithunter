#include "SaveManager.h"
#include <fstream>

SaveManager SaveManager::m_this;

void SaveManager::load() {
	fstream file;
	string path = m_path_wd + m_filename;
	file.open(path, ios::in | ios::binary);
	if (file.is_open()) {
		m_progress.clear();
		size_t size;
		file.read((char*)&size, sizeof(size_t));
		m_progress.resize(size);
		for (size_t i = 0; i < size; i++) {
			// scene
			size_t length = 0;
			file.read((char*)&length, sizeof(size_t));
			char* text = new char[length];
			file.read(text, length);
			m_progress[i].scene.insert(0, text, length);
			delete[] text;
			// time to complete
			file.read((char*)&m_progress[i].timeToComplete, sizeof(size_t));
			// grade
			int grade;
			file.read((char*)&grade, sizeof(int));
			m_progress[i].grade = (TimeTargets)grade;
		}
		file.close();
	}
	else {
		//no progress has been saved
		//do nothing
	}
}

void SaveManager::save() {
	fstream file;
	string path = m_path_wd + m_filename;
	file.open(path, ios::out | ios::binary);
	if (file.is_open()) {
		size_t size = m_progress.size();
		file.write((char*)&size, sizeof(size_t));
		for (size_t i = 0; i < m_progress.size(); i++) {
			//scene
			size_t length = m_progress[i].scene.length();
			file.write((char*)&length, sizeof(size_t));
			file.write(m_progress[i].scene.c_str(),length);
			//time to complete
			file.write((char*)&m_progress[i].timeToComplete, sizeof(size_t));
			//grade
			int grade = m_progress[i].grade;
			file.write((char*)&grade, sizeof(int));
		}
		file.close();
	}
	else {
		cout << "(SaveManager) Failed saving player progress" << endl;
	}
}

SaveManager::SaveManager() { load(); }
SaveManager::~SaveManager() {}

SaveManager* SaveManager::getInstance() { return &m_this; }

const SceneCompletion* SaveManager::getProgress(string scene) {
	SaveManager* me = getInstance();
	for (size_t i = 0; i < me->m_progress.size(); i++) {
		if (me->m_progress[i].scene == scene) {
			return &me->m_progress[i];
		}
	}
	return nullptr;
}

void SaveManager::setProgress(string scene, size_t timeToComplete, TimeTargets grade) {
	if (scene != "") {
		bool updated = false;
		bool found = false;
		SaveManager* me = getInstance();
		for (size_t i = 0; i < me->m_progress.size(); i++) {
			if (me->m_progress[i].scene == scene) {
				found = true;
				if (me->m_progress[i].timeToComplete > timeToComplete) {
					updated = true;
					// overwrite progress
					me->m_progress[i].timeToComplete = timeToComplete;
					me->m_progress[i].grade = grade;
				}
			}
		}
		if (!found) {
			updated = true;
			// add progress
			SceneCompletion p;
			p.scene = scene;
			p.timeToComplete = timeToComplete;
			p.grade = grade;
			me->m_progress.push_back(p);
		}
		if (updated)
			me->save();
	}
}

void SaveManager::resetProgression() {
	SaveManager* me = getInstance();
	me->m_progress.clear();
	me->save();
}
