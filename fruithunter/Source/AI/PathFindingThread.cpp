#include "PathFindingThread.h"

PathFindingThread PathFindingThread::m_this;


PathFindingThread::PathFindingThread() {
	ErrorLogger::log("Creating new thread");
	PathFindingThread* pft = PathFindingThread::getInstance();
	pft->m_ready = false;
	pft->m_running = true;
	pft->m_thread = new thread([this] { run(); });
}

void PathFindingThread::exitThread() {
	auto pft = PathFindingThread::getInstance();
	pft->m_mutex.lock();
	pft->m_running = false;
	pft->m_mutex.unlock();
}


PathFindingThread::~PathFindingThread() { m_thread->join(); }

PathFindingThread* PathFindingThread::getInstance() { return &m_this; }



void PathFindingThread::run() {
	// Do not print anything in this function.

	auto pft = PathFindingThread::getInstance();
	size_t counter = 0;
	size_t index = 0;
	ErrorLogger::log("Thread running");
	while (!checkVolatile(pft->m_ready)) {}
	// Thread updateLoop
	while (checkVolatile(pft->m_running)) {
		pft->m_mutex.lock();
		bool checkSize = pft->m_batch->size() > 0;
		pft->m_mutex.unlock();
		if (checkSize) {
			pft->m_mutex.lock();
			index = (index < m_batch->size() - 1) ? index + 1 : 0;
			auto object = pft->m_batch->at(index);
			bool isNull = object == nullptr;
			pft->m_mutex.unlock();
			if (!isNull)
				object->pathfinding(object->getPosition());
			// ErrorLogger::log("Thread running");
		}
	}
}

bool PathFindingThread::checkVolatile(bool& statement) {
	auto pft = PathFindingThread::getInstance();
	bool rtn;
	pft->m_mutex.lock();
	rtn = statement;
	pft->m_mutex.unlock();

	return rtn;
}

void PathFindingThread::initialize(std::vector<shared_ptr<Fruit>>& batch,
	shared_ptr<size_t> currentFrame, vector<shared_ptr<Entity>> collidables) {
	auto pft = PathFindingThread::getInstance();

	pft->m_mutex.lock();
	pft->m_batch = &batch;
	pft->m_currentFrame = currentFrame;
	pft->m_collidables = collidables;
	pft->m_ready = true;
	pft->m_mutex.unlock();
}
