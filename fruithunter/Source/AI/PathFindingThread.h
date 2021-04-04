#pragma once
#include "GlobalNamespaces.h"
#include "Fruit.h"
#include "Entity.h"
#include <memory>



class PathFindingThread {
private:
	static PathFindingThread m_this;
	thread* m_thread = nullptr; // rawptr because problem with smartptr
	mutex m_mutex;

	std::vector<shared_ptr<Fruit>>* m_batch = nullptr;
	bool m_active = true;	// is true when thread exists and should run.
	bool m_running = false; // is true when all data is set
	std::vector<float4> m_animals;

	void run();
	void exitThread();

	bool isActive();
	bool isRunning();

	PathFindingThread();
	~PathFindingThread();

public:
	static PathFindingThread* getInstance();

	void pause();

	static void lock();
	static void unlock();

	void initialize(std::vector<shared_ptr<Fruit>>& batch, std::vector<float4> animals);

};
