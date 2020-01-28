#pragma once
class State {
public:
	virtual void update() = 0;
	virtual void init() = 0;
	virtual void event() = 0;
	virtual void pause() = 0;
	virtual void play() = 0;
	virtual void draw() = 0;
	void changeState(StateHandler* statehandler, State* state) { statehandler->changeState(state); };
};
