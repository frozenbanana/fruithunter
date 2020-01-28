#include "State.hpp"

void State::changeState(StateHandler* stateHandler, State* state) { stateHandler->changeState(state); }

std::string State::getName() const { return m_name; }
