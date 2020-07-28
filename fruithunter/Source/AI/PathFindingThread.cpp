#include "PathFindingThread.h"

PathFindingThread PathFindingThread::m_this;

PathFindingThread::PathFindingThread() {
	//start thread
	if (m_thread == nullptr) {
		m_active = true;
		m_thread = new thread([this] { run(); });
	}
}

void PathFindingThread::exitThread() {
	auto pft = PathFindingThread::getInstance();
	pft->m_mutex.lock();
	pft->m_running = false;
	pft->m_active = false;
	pft->m_mutex.unlock();
	if (pft->m_thread) {
		pft->m_thread->join();
		delete pft->m_thread;
		pft->m_thread = nullptr;
	}
}

void PathFindingThread::pause() {
	auto pft = PathFindingThread::getInstance();
	pft->m_mutex.lock();
	m_running = false;
	pft->m_mutex.unlock();
}

void PathFindingThread::lock() { PathFindingThread::getInstance()->m_mutex.lock(); }

void PathFindingThread::unlock() { PathFindingThread::getInstance()->m_mutex.unlock(); }

PathFindingThread::~PathFindingThread() { exitThread(); }

PathFindingThread* PathFindingThread::getInstance() { return &m_this; }

void PathFindingThread::run() {
	// Do not print anything in this function.

	auto pft = PathFindingThread::getInstance();
	int index = 0; // index on fruit array to process

	// wait until initialized.
	//while (!checkVolatile(pft->m_ready)) {}
	// Thread updateLoop
	while (isActive()) {
		while (isRunning()) {
			pft->m_mutex.lock();
			index = (index < m_batch->size() - 1) ? index + 1 : 0; // fruit index
			if (pft->m_batch->size() > 0) {
				auto object = pft->m_batch->at(index);
				if (object.get() != nullptr)
					object->pathfinding(object->getPosition(), &m_animals);
			}
			pft->m_mutex.unlock();
		}
	}
}

bool PathFindingThread::isActive() { 
	auto pft = PathFindingThread::getInstance();
	bool ret;
	pft->m_mutex.lock(); 
	ret = m_active;
	pft->m_mutex.unlock(); 
	return ret;
}

bool PathFindingThread::isRunning() {
	auto pft = PathFindingThread::getInstance();
	bool ret;
	pft->m_mutex.lock();
	ret = m_running;
	pft->m_mutex.unlock();
	return ret;
}

void PathFindingThread::initialize(std::vector<shared_ptr<Fruit>>& batch, std::vector<float4> animals) {

	auto pft = PathFindingThread::getInstance();
	pft->m_mutex.lock();
	//set data
	pft->m_batch = &batch;
	pft->m_animals = animals;
	// make available for processing
	pft->m_running = true;
	pft->m_mutex.unlock();
}
