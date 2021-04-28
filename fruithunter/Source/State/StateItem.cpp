#include "StateItem.h"
#include "StateStack.h"

StateItem::StateItem(State state) {
	m_state = state;
}

StateItem::~StateItem() {}

StateItem::State StateItem::getState() const { return m_state; }

string StateItem::getStateAsString() const { 
	return m_stateStrings[m_state]; 
}

StateItem::Request StateItem::popRequest() {
	Request ret = m_request;
	m_request = Request();
	return ret;
}

void StateItem::push(StateItem::State state) { 
	m_request.type = Request::Type::PUSH;
	m_request.state = state;
}

void StateItem::pop(bool restart) {
	m_request.type = Request::Type::POP;
	m_request.restart = restart;
}

void StateItem::pop(StateItem::State state, bool restart) {
	m_request.type = Request::Type::POP;
	m_request.state = state;
	m_request.restart = restart;
}
