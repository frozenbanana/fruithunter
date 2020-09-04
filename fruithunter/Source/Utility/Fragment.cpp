#include "Fragment.h"
#include <string>

size_t Fragment::m_id_counter = 0;

Fragment::Fragment(Type type, std::string description) { 
	m_id = nextID();
	m_type = type;
	if (description == "")
		m_description = getTypeAsString() + std::to_string(m_id);
	else
		m_description = description;
}

Fragment::Fragment(const Fragment& other) { operator=(other); }

Fragment::~Fragment() {}

void Fragment::operator=(const Fragment& other) { m_type = other.m_type;
	m_description = other.m_description;
	m_id = nextID();
}

void Fragment::rename(std::string description) { m_description = description; }

void Fragment::changeType(Type type) {
	m_type = type;
	m_description = getTypeAsString() + std::to_string(m_id);
}

FragmentID Fragment::getID() const { return m_id; }

std::string Fragment::getDescription() const { return m_description; }

Fragment::Type Fragment::getType() const { return m_type; }

std::string Fragment::getTypeAsString() const { return m_typeAsString[m_type]; }

std::string Fragment::getFullDescription() const {
	return getDescription() + "(" + std::to_string(getID()) + ", " + getTypeAsString() + ")";
}

size_t Fragment::nextID() { 
	m_id_counter++;
	return m_id_counter; 
}