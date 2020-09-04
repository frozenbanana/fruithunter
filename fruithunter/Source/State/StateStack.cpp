#include "StateStack.h"

StateStack StateStack::m_this;

StateItem* StateStack::CurrentState() const { return m_stack.back().get(); }

void StateStack::handleRequest() {
	if (!isEmpty()) {
		StateItem* item = CurrentState();
		StateItem::Request request = item->popRequest();
		if (request.type == StateItem::Request::Type::PUSH) {
			push(request.state);
		}
		else if (request.type == StateItem::Request::Type::POP) {
			if (request.state == StateItem::State::Size)
				pop(request.restart);
			else
				pop(request.state, request.restart);
		}
		else {
			// Empty request
		}
	}
}

void StateStack::NotifyStack(bool pushed) const {
	string msg = "State Stack: ";
	for (size_t i = 0; i < m_stack.size(); i++) {
		msg += m_stack[i]->getStateAsString();
		if (i < m_stack.size()-1)
			msg += " -> ";
	}
	cout << msg << (pushed?" (Pushed)":" (Popped)") << endl;
}

StateStack* StateStack::getInstance() { return &m_this; }

void StateStack::update() {
	if (!isEmpty())
		CurrentState()->update();
}

void StateStack::draw() {
	if (!isEmpty())
		CurrentState()->draw();
}

void StateStack::push(StateItem::State state) {
	shared_ptr<StateItem> item;
	switch (state) {
	case StateItem::PlayState:
		item = make_shared<PlayState>();
		break;
	case StateItem::PauseState:
		item = make_shared<PauseState>();
		break;
	case StateItem::SettingState:
		item = make_shared<SettingsState>();
		break;
	case StateItem::LevelSelectState:
		item = make_shared<LevelSelectState>();
		break;
	case StateItem::MainState:
		item = make_shared<MainState>();
		break;
	case StateItem::EndRoundState:
		item = make_shared<EndRoundState>();
		break;
	case StateItem::EditorState:
		item = make_shared<EditorState>();
		break;
	}
	if (item.get() != nullptr) {
		//pause
		if (m_stack.size() > 0)
			m_stack.back()->pause();
		//insert
		m_stack.push_back(item);
		//init
		m_stack.back()->init();
		//play
		m_stack.back()->play();
		//notify
		NotifyStack(true);
	}
}

void StateStack::pop(bool restart) {
	if (m_stack.size() > 0)
		m_stack.pop_back();
	if (m_stack.size() > 0) {
		m_stack.back()->play();
		if (restart) {
			m_stack.back()->restart();
		}
	}
	NotifyStack(false);
}

void StateStack::pop(StateItem::State state, bool restart) {
	while (m_stack.size() > 0 && m_stack.back()->getState() != state)
		m_stack.pop_back();
	if (m_stack.size() > 0) {
		m_stack.back()->play();
		if (restart)
			m_stack.back()->restart();
	}
	NotifyStack(false);
}

bool StateStack::isEmpty() const { return m_stack.size() == 0; }
