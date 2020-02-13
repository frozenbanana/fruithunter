#include "MeshRepository.h"

MeshRepository MeshRepository::m_this;

MeshRepository::MeshRepository() {}

MeshRepository::~MeshRepository() {
	for (size_t i = 0; i < m_repository.size(); i++) {
		m_repository[i].reset();
	}
}

bool MeshRepository::add(string OBJ_Name) {
	shared_ptr<Mesh> sh_ptr = make_shared<Mesh>();
	bool state = sh_ptr->load(OBJ_Name);
	if (state) {
		m_repository.push_back(sh_ptr);
		ErrorLogger::log("Loaded Mesh: " + OBJ_Name);
	}
	else {
		ErrorLogger::logWarning(HRESULT(),"Failed loading Mesh: " + OBJ_Name);
	}
	return state;
}

MeshRepository* MeshRepository::getInstance() { return &m_this; }

shared_ptr<Mesh> MeshRepository::get(string OBJ_Name) {
	if (OBJ_Name != "") {
		MeshRepository* mr = MeshRepository::getInstance();
		// find mesh ...
		for (size_t i = 0; i < mr->m_repository.size(); i++) {
			if (mr->m_repository[i].get()->getName() == OBJ_Name) {
				return mr->m_repository[i];
			}
		}
		// or load mesh
		bool state = mr->add(OBJ_Name);
		if (state) {
			return mr->m_repository.back();
		}
		else {
			return shared_ptr<Mesh>();
		}
	}
}
