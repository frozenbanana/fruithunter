#pragma once
#include "GlobalNamespaces.h"
#include "Fruit.h"
#include "Entity.h"
#include <memory>



class PathFindingThread {

public:
	static PathFindingThread* getInstance();

	std::shared_ptr<size_t> m_currentFrame;
	// rawptr because problem with smartptr
	thread* m_thread = nullptr;
	mutex m_mutex;
	std::vector<shared_ptr<Fruit>>* m_batch;
	std::vector<shared_ptr<Entity>> m_collidables;
	void initialize(std::vector<shared_ptr<Fruit>>& batch, shared_ptr<size_t> currentFrame,
		vector<shared_ptr<Entity>> collidables);
	~PathFindingThread();

	void exitThread();

private:
	PathFindingThread();
	static PathFindingThread m_this;
	bool m_ready, m_running;


	bool checkVolatile(bool& statement);



	void run();
};
