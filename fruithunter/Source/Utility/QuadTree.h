#pragma once
#include "GlobalNamespaces.h"
#include "ErrorLogger.h"

#define COUNT_SPLIT 2

template <typename Element> class Node {
public:
	struct ElementPart {
		float2 position, size;
		Element* ptr;
		ElementPart(float2 _position = float2(0, 0), float2 _size = float2(0, 0),
			Element* _ptr = nullptr) {
			position = _position;
			size = _size;
			ptr = _ptr;
		}
	};

private:
	size_t m_layerMax;
	size_t m_layer;
	bool expanded = false;
	Node* m_children[4] = { nullptr };
	float2 m_position, m_size;

	vector<ElementPart> m_elements;

	//-- Functions --
	static bool bbIntersection(float2 bPos1, float2 bSize1, float2 bPos2, float2 bSize2);

public:
	void log(int stage = 0);

	void split();
	void add(const ElementPart& element);
	void remove(Element* ptr);

	Node(float2 position = float2(0, 0), float2 size = float2(0, 0), size_t layer = 0, size_t layerMax = 1);
};


template <typename Element>
inline bool Node<Element>::bbIntersection(
	float2 bPos1, float2 bSize1, float2 bPos2, float2 bSize2) {
	return (bPos1.x < bPos2.x + bSize2.x && bPos1.y < bPos2.y + bSize2.y &&
			bPos1.x + bSize1.x > bPos2.x && bPos1.y + bSize1.y > bPos2.y);
}

//string v2ToString(float2 v) { return to_string(v.x) + " - " + to_string(v.y); }

template <typename Element> inline void Node<Element>::log(int stage) {
	string str = "";
	for (size_t i = 0; i < stage; i++)
		str += "-";
	str += 
		"Node: " + (stage == 0 ? "CORE" : to_string(stage)) +
		", Size: " + to_string(m_elements.size()) +
		", Expanded: " + (expanded ? "true" : "false"
	);
	// str += "\n";
	ErrorLogger::log(str);
	if (expanded) {
		for (size_t i = 0; i < 4; i++)
			m_children[i]->log(stage + 1);
	}
}

template <typename Element> inline void Node<Element>::split() {
	float2 halfSize = m_size / 2.f;
	m_children[0] = new Node(m_position + float2(0.f * halfSize.x, 0.f * halfSize.y), halfSize, m_layerMax);
	m_children[1] = new Node(m_position + float2(1.f * halfSize.x, 0.f * halfSize.y), halfSize);
	m_children[2] = new Node(m_position + float2(0.f * halfSize.x, 1.f * halfSize.y), halfSize);
	m_children[3] = new Node(m_position + float2(1.f * halfSize.x, 1.f * halfSize.y), halfSize);

	expanded = true;

	// add existing elements to children
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < m_elements.size(); j++)
			m_children[i]->add(m_elements[j]);
	}
}

template <typename Element>
inline void Node<Element>::add(const ElementPart& element) {
	if (bbIntersection(element.position, element.size, m_position, m_size)) {
		//split if to big
		if (m_layer < m_layerMax && m_elements.size() > COUNT_SPLIT-1)
			split();
		// add
		m_elements.push_back(element);
		// add for all children
		if (expanded) {
			for (size_t i = 0; i < 4; i++)
				m_children[i]->add(element);
		}
	}
}
template <typename Element> inline void Node<Element>::remove(Element* ptr) {
	for (size_t i = 0; i < m_elements.size(); i++) {
		if (m_elements[i].ptr == ptr) {
			//remove
			m_elements.erase(m_elements.begin() + i);
			if (expanded) {
				for (size_t j = 0; j < 4; j++)
					m_children[j].remove(ptr);
			}
			break;
		}
	}
}
template <typename Element>
inline Node<Element>::Node(float2 position, float2 size, size_t layer, size_t layerMax) {
	m_position = position;
	m_size = size;
	m_layer = layer;
	m_layerMax = layerMax;
}
