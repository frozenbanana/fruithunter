#include "PathFindingThread.h"
#include <chrono>

PathFindingThread PathFindingThread::m_this;


PathFindingThread::PathFindingThread() {
	// auto pft = PathFindingThread::getInstance();

}

void PathFindingThread::exitThread() {
	auto pft = PathFindingThread::getInstance();
	pft->m_mutex.lock();
	pft->m_running = false;
	pft->m_ready = true;
	pft->m_mutex.unlock();
	pft->m_thread->join();
	if (pft->m_thread) {
		delete pft->m_thread;
	}
	pft->m_thread = nullptr;
}
//
//void PathFindingThread::addPathToWork(std::vector<float3*> &path) {
//	m_sizeLock.lock();
//	m_pathVector.push_back(path);
//	m_sizeLock.unlock();
//}

void PathFindingThread::addFruit(Fruit* &fruit) {
	//m_sizeLock.lock();
	auto pft = PathFindingThread::getInstance();
	pft->m_batch.push_back(fruit);
	//m_sizeLock.unlock();
}


PathFindingThread::~PathFindingThread() {
	
}

PathFindingThread* PathFindingThread::getInstance() { return &m_this; }



void PathFindingThread::run() {
	// Do not print anything in this function.

	auto pft = PathFindingThread::getInstance();
	size_t counter = 0;
	int index = 0;

	// wait until initialized.
	while (!checkVolatile(pft->m_ready)) {}
	// Thread updateLoop
	while (checkVolatile(pft->m_running)) {
		pft->m_sizeLock.lock();
		if (!pft->m_batch.empty()) {
			// pft->.lock();
			auto object = pft->m_batch.front();
			object->pathfinding(object->getPosition(), m_animals);
			
			//object.
			pft->m_batch.pop_front();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		pft->m_sizeLock.unlock();
		//pft->m_sizeLock.lock();
		//if (pft->m_pathVector.size() > 0) {
		//	auto path = pft->m_pathVector.front();
		//	AI::pathfinding(path);
		//}
		//pft->m_sizeLock.unlock();
		
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
	shared_ptr<size_t> currentFrame, EntityRepository &collidables, std::vector<float4> animals) {
	auto pft = PathFindingThread::getInstance();
	pft->m_ready = false;
	pft->m_running = true;
	pft->m_thread = new thread([this] { run(); });
	pft->m_mutex.lock();
	pft->m_currentFrame = currentFrame;
	pft->m_collidables = &collidables;
	pft->m_ready = true;
	pft->m_animals = animals;
	pft->m_mutex.unlock();
}
