#include "PathFindingThread.h"

PathFindingThread PathFindingThread::m_this;


PathFindingThread::PathFindingThread() {
	ErrorLogger::log("Creating new thread");
	m_ready = false;
	m_running = true;
	m_thread = new thread([this] { run(); });
}


PathFindingThread::~PathFindingThread() {
	m_mutex.lock();
	m_running = false;
	m_mutex.unlock();
	m_thread->join();
}

PathFindingThread* PathFindingThread::getInstance() {
	return & m_this;
}



void PathFindingThread::run() {

	size_t counter = 0;
	size_t index = 0;

	while (!checkVolitale(m_ready)) {}
	// Thread updateLoop
	while (checkVolitale(m_running)) {
		/*m_mutex.lock();
		counter += (*m_currentFrame % 10 == 0) ? 1 : 0;
		m_mutex.unlock();

		if (checkVolitale(m_batch.at(index)->giveNewPath())) {
			float3 pathStart = m_batch.at(index)->getPosition();
			m_batch.at(index)->pathfinding(pathStart, m_collidables, m_mutex);
		}
*/
		ErrorLogger::log("Thread running");
		index = (index < m_batch.size()) ? index + 1 : 0;
	}
}

bool PathFindingThread::checkVolitale(bool &statement) {
	bool rtn;
	m_mutex.lock();
	rtn = statement;
	m_mutex.unlock();

	return rtn;
}

void PathFindingThread::initialize(std::vector<shared_ptr<Fruit>> batch,
	shared_ptr<size_t> currentFrame, vector<shared_ptr<Entity>> collidables) {
	m_mutex.lock();
	m_batch = batch;
	m_currentFrame = currentFrame;
	m_collidables = collidables;

	m_ready = true;

	m_mutex.unlock();
}
