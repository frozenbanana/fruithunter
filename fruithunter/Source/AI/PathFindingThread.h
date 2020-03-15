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
	mutex m_sizeLock;
	std::list<Fruit*> m_batch;
	//std::vector<AI::Path*> m_pathVector;
	std::vector<shared_ptr<Entity>> m_collidables;
	void initialize( shared_ptr<size_t> currentFrame,
		vector<shared_ptr<Entity>> collidables);
	~PathFindingThread();

	void exitThread();
	//void addPathToWork(std::vector<AI::Path*> &path);
	void addFruit(Fruit* &fruit);

private:
	PathFindingThread();
	static PathFindingThread m_this;
	bool m_ready, m_running;


	bool checkVolatile(bool& statement);
	


	void run();
};
