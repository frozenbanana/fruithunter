#pragma once
#include "GlobalNamespaces.h"
#include "Fruit.h"
#include "Entity.h"
#include "EntityRepository.h"
#include <memory>



class PathFindingThread {

public:
	static PathFindingThread* getInstance();

	// rawptr because problem with smartptr
	thread* m_thread = nullptr;
	mutex m_mutex;
	//std::vector<shared_ptr<Entity>> m_collidables;
	void initialize(std::vector<shared_ptr<Fruit>>& batch, shared_ptr<size_t> currentFrame,
		EntityRepository& collidables, std::vector<float4> animals);
	~PathFindingThread();

	void exitThread();
	//void addPathToWork(std::vector<AI::Path*> &path);
	void addFruit(Fruit* &fruit);

	EntityRepository* m_collidables;
private:
	PathFindingThread();
	std::shared_ptr<size_t> m_currentFrame;
	std::vector<shared_ptr<Fruit>>* m_batch;
	static PathFindingThread m_this;
	bool m_ready, m_running;
	std::vector<float4> m_animals;


	bool checkVolatile(bool& statement);
	


	void run();
};
