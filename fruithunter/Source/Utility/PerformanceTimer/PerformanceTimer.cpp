#include "PerformanceTimer.h"
#include "ErrorLogger.h"

PerformanceTimer PerformanceTimer::m_this;

vector<string> PerformanceTimer::getPathFromQueue() const {
	vector<string> path;
	if (m_queue.size() > 1) {
		path.resize(m_queue.size() - 1);
		for (size_t i = 0; i < m_queue.size() - 1; i++) {
			path[i] = m_queue[(m_queue.size() - 1) - i].m_title;
		}
	}
	return path;
}

PerformanceTimer* PerformanceTimer::getInstance() { return &m_this; }

void PerformanceTimer::start(string title, TimeState state) {
	m_this.m_queue.push_back(QueueItem(title, state));
}

void PerformanceTimer::stop() {
	vector<string> path = m_this.getPathFromQueue();
	QueueItem item = m_this.m_queue.back();
	m_this.m_queue.pop_back();
	m_this.m_source.place(path, item.getTimeDifference(), item.m_state);
}

void PerformanceTimer::log() {
	string str = "";
	m_this.m_source.toString(str, 0, 100);
	ErrorLogger::log(str);
}

void PerformanceTimer::logToFile(string filename) {
	if (filename != "") {
		fstream file;
		string path = m_preFilePath + filename + m_postFileEndings;
		file.open(path, ios::out);
		if (file.is_open()) {
			string str = "";
			m_source.toString(str, 0, 100);
			file << str;
			file.close();
		}
		else {
			ErrorLogger::logWarning(
				HRESULT(), "(PerformanceTimer) Failed writing to file: " + path);
		}
	}
}

PerformanceTimer::PerformanceTimer() {
	m_source = Folder(m_sourceTitle);
	start(m_source.m_title, TimeState::state_accumulate);
}

PerformanceTimer::~PerformanceTimer() {
	stop();
	logToFile(m_runtimeAnalysisFilename);
}

double PerformanceTimer::Folder::getTotalTime() const {
	if (m_state == TimeState::state_accumulate)
		return m_collectedTime;
	else if (m_state == TimeState::state_average)
		return m_collectedTime * m_count;
	return m_collectedTime;
}

string PerformanceTimer::Folder::stateToString() const {
	if (m_state == TimeState::state_accumulate)
		return "Accumulate";
	else if (m_state == TimeState::state_average)
		return "Average";
	return "Null";
}

void PerformanceTimer::Folder::place(vector<string>& path, double time, TimeState state) {
	if (path.size() > 0) {
		string title = path.back();
		path.pop_back();
		bool found = false;
		for (size_t i = 0; i < m_subFolders.size(); i++) {
			if (m_subFolders[i].m_title == title) {
				found = true;
				m_subFolders[i].place(path, time, state);
				break;
			}
		}
		if (!found) {
			m_subFolders.push_back(Folder(title));
			m_subFolders.back().place(path, time, state);
		}
	}
	else {
		m_state = state;
		increment(time, state);
	}
}

void PerformanceTimer::Folder::increment(double time, TimeState& state) {
	m_count++;
	if (state == TimeState::state_accumulate)
		m_collectedTime += time;
	else if (state == TimeState::state_average)
		m_collectedTime = (m_collectedTime * (double)(m_count - 1) + time) / (double)m_count;
}

void PerformanceTimer::Folder::toString(string& retStr, size_t level, float percentage) {
	string preStr = "";
	for (size_t i = 0; i < level; i++)
		preStr += "|  ";

	string str = preStr + m_title + "(" + stateToString() +
				 "): " + to_string((long unsigned)m_collectedTime) + " ms | " +
				 to_string(percentage) + "%";

	retStr += str + "\n";
	for (size_t i = 0; i < m_subFolders.size(); i++) {
		m_subFolders[i].toString(retStr,
			level + 1, 100.f * (float)(m_subFolders[i].getTotalTime() / getTotalTime()));
	}
	if (m_subFolders.size() > 0)
		retStr += preStr + "\n";
}

PerformanceTimer::Folder::Folder(string title) { m_title = title; }

float PerformanceTimer::QueueItem::getTimeDifference() const { return (float)(clock() - m_time); }
