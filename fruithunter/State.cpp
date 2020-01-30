#include "State.hpp"
#include "StateHandler.hpp"

void State::changeState(int state) { StateHandler::getInstance()->changeState((StateHandler::States)state); }

std::string State::getName() const { return m_name; }
