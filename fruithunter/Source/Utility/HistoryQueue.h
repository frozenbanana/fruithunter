#pragma once
#include <iostream>
#include <vector>
#include <WRL/client.h>

using namespace std;

template <typename STRUCT> class HistoryQueue {
private:
	bool m_pushing = false;
	unsigned int m_index = 0;
	vector<shared_ptr<STRUCT>> m_queue;

	unsigned int m_stackLimit;

	/* Overwrites class from sample */
	virtual void hq_apply(shared_ptr<STRUCT> sample) = 0;

	/* Fetches current state of class */
	virtual shared_ptr<STRUCT> hq_fetch() const = 0;

public:
	void hq_push_begin();
	void hq_push_end();
	bool hq_undo();
	bool hq_redo();
	void hq_reset();

	HistoryQueue<STRUCT> operator=(const HistoryQueue<STRUCT>& other);

	HistoryQueue(unsigned int stackLimit);
};

template <typename STRUCT> inline void HistoryQueue<STRUCT>::hq_push_begin() {
	if (!m_pushing) {
		m_pushing = true;
		if (m_queue.size() == 0)
			m_queue.push_back(hq_fetch());
	}
}

template <typename STRUCT>
inline void HistoryQueue<STRUCT>::hq_push_end() { // erase potential redos
	if (m_pushing) {
		m_pushing = false;

		size_t position = m_index + 1;
		if (m_queue.size() != position)
			m_queue.resize(position);

		// push mesh
		m_queue.push_back(hq_fetch());
		m_index++;
		if (m_queue.size() > m_stackLimit) {
			m_queue.erase(m_queue.begin()); // remove first if to many samples
			m_index--;
		}
	}
}

template <typename STRUCT> inline bool HistoryQueue<STRUCT>::hq_undo() {
	if (!m_pushing) {
		if (m_index > 0) {
			m_index--; // backtrack

			hq_apply(m_queue[m_index]);

			return true;
		}
	}
	return false;
}

template <typename STRUCT> inline bool HistoryQueue<STRUCT>::hq_redo() {
	if (!m_pushing) {
		size_t position = m_index + 1;
		if (position < m_queue.size()) {
			m_index++;

			hq_apply(m_queue[m_index]);

			return true;
		}
	}
	return false;
}

template <typename STRUCT> inline void HistoryQueue<STRUCT>::hq_reset() {
	m_queue.clear();
	m_index = 0;
	m_pushing = false;
}

template <typename STRUCT>
inline HistoryQueue<STRUCT> HistoryQueue<STRUCT>::operator=(const HistoryQueue<STRUCT>& other) {
	m_stackLimit = other.m_stackLimit;
	m_pushing = other.m_pushing;
	m_index = other.m_index;
	m_queue.resize(other.m_queue.size());
	for (size_t i = 0; i < m_queue.size(); i++) {
		m_queue[i] = make_shared<STRUCT>(*other.m_queue[i].get());
	}
}

template <typename STRUCT> inline HistoryQueue<STRUCT>::HistoryQueue(unsigned int stackLimit) {
	m_stackLimit = stackLimit;
}
