#pragma once
#include "Mesh.h"
class MeshRepository {
private:
	static MeshRepository m_this;
	vector<shared_ptr<Mesh>> m_repository;

	//--Functions--

	bool add(string OBJ_Name);

	MeshRepository();
	~MeshRepository();

public:
	static MeshRepository* getInstance();
	
	static shared_ptr<Mesh> get(string OBJ_Name);

};
