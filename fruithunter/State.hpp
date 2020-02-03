#pragma once
#include <iostream>

class StateHandler;

class State {
public:
	virtual void update() = 0;
	virtual void initialize() = 0;
	virtual void handleEvent(int event) = 0;
	virtual void pause() = 0;
	virtual void play() = 0;
	virtual void draw() = 0;
	void changeState(int state);
	std::string getName() const;
	virtual ~State(){};

	virtual ~State() {};

protected:
	State() {};
	
	std::string m_name = "Unset";
};
