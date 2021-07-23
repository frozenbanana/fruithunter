#pragma once
#include "States.h"
#include <vector>

class StateStack {
private:
	static StateStack m_this;
	
	vector<shared_ptr<StateItem>> m_stack;

	bool m_pushedBeforeUpdate = false;

	StateItem* CurrentState() const;
	void NotifyStack(bool pushed) const;

public:
	static StateStack* getInstance();

	void handleRequest();
	void update(double dt);
	void draw();

	/* Add state to stack*/
	void push(StateItem::State state);
	/* Go to previous state */
	void pop(bool restart);
	/* Jump back to state */
	void pop(StateItem::State state, bool restart);

	bool isEmpty() const;

};
