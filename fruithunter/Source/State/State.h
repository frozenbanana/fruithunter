#pragma once
#include <iostream>
#include "GlobalNamespaces.h"

class StateHandler;

__declspec(align(16)) class State {
public:
	virtual void update() = 0;
	virtual void initialize() = 0;
	virtual void handleEvent() = 0;
	virtual void pause() = 0;
	virtual void play() = 0;
	virtual void draw() = 0;
	void changeState(int state);
	std::string getName() const;
	virtual ~State(){};
	// To avoid object allocated on the heap may not be aligned 16 - warning
	// See:
	// https://stackoverflow.com/questions/20104815/warning-c4316-object-allocated-on-the-heap-may-not-be-aligned-16
	void* operator new(size_t i) { return _mm_malloc(i, 16); }
	void operator delete(void* p) { _mm_free(p); }

protected:
	State(){};

	std::string m_name = "Unset";
};
