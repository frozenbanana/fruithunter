#pragma once
#include "GlobalNamespaces.h"
#include "ErrorLogger.h"

#define COUNT_SPLIT 4

enum bbFrustumState { State_Inside, State_Inbetween, State_Outside };
template <typename Element> class QuadTree {
private:
	struct ElementPart {
		size_t index;
		float3 position, size;
		bool fetched = false;
		Element element;
		ElementPart(float3 _position, float3 _size, const Element& _element) {
			position = _position;
			size = _size;
			element = _element;
		}
		ElementPart() {}
	};
	class Node {
	private:
		size_t m_layerMax;
		size_t m_layer;
		bool expanded = false;
		shared_ptr<Node> m_children[4];
		float3 m_position, m_size;

		vector<ElementPart*> m_elements;

		//-- Functions --
		static bool bbIntersection(float3 bPos1, float3 bSize1, float3 bPos2, float3 bSize2);
		bbFrustumState boxInsideFrustum(
			float3 boxPos, float3 boxSize, const vector<FrustumPlane>& planes);

	public:
		static int test;
		size_t getElementCount() const;
		void log(int level = 0);

		void split();
		bool add(ElementPart* elementPart);
		void remove(const ElementPart* elementPart);

		void cullElements(
			const vector<FrustumPlane>& planes, vector<Element*>& elements, size_t& count);
		void cullElements(const CubeBoundingBox& bb, vector<Element*>& elements, size_t& count);
		void forEach_cullElements(const vector<FrustumPlane>& planes, vector<bool>& partsEnabled,
			void (*function_onEach)(Element* ptr));

		Node(float3 position = float3(0, 0, 0), float3 size = float3(0, 0, 0), size_t layerMax = 1,
			size_t layer = 0, size_t reserve = 0);
		~Node();
	} m_node;
	vector<shared_ptr<ElementPart>> m_elementParts;

	void resetFetchState();

public:
	void log();

	void add(float3 position, float3 size, const Element& element);
	void add(
		float3 lCenterPosition, float3 lHalfSize, float4x4 worldMatrix, const Element& element);
	void remove(Element& element);
	vector<Element*> cullElements(const vector<FrustumPlane>& planes);
	vector<Element*> cullElements(const CubeBoundingBox& bb);
	void foreach_cullElements(const vector<FrustumPlane>& planes, void (*onEach)(Element*));

	void initilize(float3 position, float3 size, size_t layerMax);
	void reset();
	void reserve(size_t size);

	QuadTree(float3 position = float3(0, 0, 0), float3 size = float3(0, 0, 0), size_t layerMax = 1);
	~QuadTree();
};

template <typename Element>
inline bool QuadTree<Element>::Node::bbIntersection(
	float3 bPos1, float3 bSize1, float3 bPos2, float3 bSize2) {
	return (bPos1.x < bPos2.x + bSize2.x && bPos1.y < bPos2.y + bSize2.y &&
			bPos1.z < bPos2.z + bSize2.z && bPos1.x + bSize1.x > bPos2.x &&
			bPos1.y + bSize1.y > bPos2.y && bPos1.z + bSize1.z > bPos2.z);
}

template <typename Element>
inline bbFrustumState QuadTree<Element>::Node::boxInsideFrustum(
	float3 boxPos, float3 boxSize, const vector<FrustumPlane>& planes) {

	// normalized box points
	float3 boxPoints[8] = { float3(0, 0, 0), float3(1, 0, 0), float3(1, 0, 1), float3(0, 0, 1),
		float3(0, 1, 0), float3(1, 1, 0), float3(1, 1, 1), float3(0, 1, 1) };
	// transform points to world space
	for (size_t i = 0; i < 8; i++) {
		boxPoints[i] = boxPos + boxPoints[i] * boxSize;
	}
	// for each plane
	bool inbetween = false;
	float largestDot;
	for (size_t plane_i = 0; plane_i < planes.size(); plane_i++) {
		float3 boxDiagonalPoint1, boxDiagonalPoint2;
		largestDot = -1;
		float3 p1, p2, pn;
		float dot, min, max, temp;
		// find diagonal points
		for (size_t j = 0; j < 4; j++) {
			p1 = boxPoints[j];
			p2 = boxPoints[4 + (j + 2) % 4];
			pn = p1 - p2;
			pn.Normalize();
			dot = abs(pn.Dot(planes[plane_i].m_normal));
			if (dot > largestDot) {
				largestDot = dot;
				boxDiagonalPoint1 = p1;
				boxDiagonalPoint2 = p2;
			}
		}
		// compare points
		min = (boxDiagonalPoint1 - planes[plane_i].m_position).Dot(planes[plane_i].m_normal);
		max = (boxDiagonalPoint2 - planes[plane_i].m_position).Dot(planes[plane_i].m_normal);
		if (min > max) {
			// switch
			temp = max;
			max = min;
			min = temp;
		}
		if (min > 0) {
			// outside
			return State_Outside;
		}
		else if (max < 0) {
			// inside
		}
		else {
			// inbetween
			inbetween = true;
		}
	}
	if (inbetween)
		return State_Inbetween;
	else
		return State_Inside;
}

template <typename Element> inline size_t QuadTree<Element>::Node::getElementCount() const {
	return m_elements.size();
}

template <typename Element> inline void QuadTree<Element>::Node::log(int level) {
	string str = "";
	for (size_t i = 0; i < level; i++)
		str += "-";
	str += "Node: " + (level == 0 ? "CORE" : to_string(level)) +
		   ", Size: " + to_string(m_elements.size()) +
		   ", Expanded: " + (expanded ? "true" : "false");
	// str += "\n";
	ErrorLogger::log(str);
	if (expanded) {
		for (size_t i = 0; i < 4; i++)
			m_children[i]->log(level + 1);
	}
}

template <typename Element> inline void QuadTree<Element>::Node::split() {
	if (!expanded) {
		expanded = true;

		float3 halfSize = m_size;
		halfSize.x /= 2.f;
		halfSize.z /= 2.f;
		size_t cap = m_elements.capacity() / 4;

		m_children[0] = make_shared<Node>(
			m_position + halfSize * float3(0.f, 0.f, 0.f), halfSize, m_layerMax, m_layer + 1, cap);
		m_children[1] = make_shared<Node>(
			m_position + halfSize * float3(1.f, 0.f, 0.f), halfSize, m_layerMax, m_layer + 1, cap);
		m_children[2] = make_shared<Node>(
			m_position + halfSize * float3(0.f, 0.f, 1.f), halfSize, m_layerMax, m_layer + 1, cap);
		m_children[3] = make_shared<Node>(
			m_position + halfSize * float3(1.f, 0.f, 1.f), halfSize, m_layerMax, m_layer + 1, cap);

		// add existing elements to children
		for (size_t i = 0; i < 4; i++) {
			for (size_t j = 0; j < m_elements.size(); j++)
				m_children[i]->add(m_elements[j]);
		}
	}
}

template <typename Element> inline bool QuadTree<Element>::Node::add(ElementPart* elementPart) {
	if (bbIntersection(elementPart->position, elementPart->size, m_position, m_size)) {
		// split if to big
		if (!expanded && m_layer < m_layerMax && m_elements.size() > COUNT_SPLIT - 1)
			split();
		// add
		m_elements.push_back(elementPart);
		// add for all children
		if (expanded) {
			for (size_t i = 0; i < 4; i++)
				m_children[i]->add(elementPart);
		}
		return true;
	}
	return false;
}
template <typename Element>
inline void QuadTree<Element>::Node::remove(const ElementPart* elementPart) {
	for (size_t i = 0; i < m_elements.size(); i++) {
		if (m_elements[i] == elementPart) {
			// remove
			m_elements.erase(m_elements.begin() + i);
			// remove from children
			if (expanded) {
				for (size_t j = 0; j < 4; j++)
					m_children[j]->remove(elementPart);
			}
			break;
		}
	}
}

template <typename Element>
inline void QuadTree<Element>::Node::cullElements(
	const vector<FrustumPlane>& planes, vector<Element*>& elements, size_t& count) {

	bbFrustumState state = boxInsideFrustum(m_position, m_size, planes);
	count++;
	switch (state) {
	case bbFrustumState::State_Inside:
		// full hit, add all elements to the list
		for (size_t i = 0; i < m_elements.size(); i++) {
			if (!m_elements[i]->fetched) {
				m_elements[i]->fetched = true;
				elements.push_back(&m_elements[i]->element);
			}
		}
		break;
	case bbFrustumState::State_Outside:
		// miss, add none
		break;
	case bbFrustumState::State_Inbetween:
		// partial hit
		if (expanded) {
			// add children elements
			for (size_t i = 0; i < 4; i++)
				m_children[i]->cullElements(planes, elements, count);
		}
		else {
			// add all elements to the list
			for (size_t i = 0; i < m_elements.size(); i++) {
				if (!m_elements[i]->fetched) {
					m_elements[i]->fetched = true;
					elements.push_back(&m_elements[i]->element);
				}
			}
		}
		break;
	}
}

template <typename Element>
inline void QuadTree<Element>::Node::cullElements(
	const CubeBoundingBox& bb, vector<Element*>& elements, size_t& count) {
	bbFrustumState state = bbIntersection(m_position, m_size, bb.m_position, bb.m_size)
							   ? bbFrustumState::State_Inbetween
							   : bbFrustumState::State_Outside;

	count++;
	switch (state) {
	case bbFrustumState::State_Inside:
		// full hit, add all elements to the list
		for (size_t i = 0; i < m_elements.size(); i++) {
			if (!m_elements[i]->fetched) {
				m_elements[i]->fetched = true;
				elements.push_back(&m_elements[i]->element);
			}
		}
		break;
	case bbFrustumState::State_Outside:
		// miss, add none
		break;
	case bbFrustumState::State_Inbetween:
		// partial hit
		if (expanded) {
			// add children elements
			for (size_t i = 0; i < 4; i++)
				m_children[i]->cullElements(bb, elements, count);
		}
		else {
			// add all elements to the list
			for (size_t i = 0; i < m_elements.size(); i++) {
				if (!m_elements[i]->fetched) {
					m_elements[i]->fetched = true;
					elements.push_back(&m_elements[i]->element);
				}
			}
		}
		break;
	}
}

template <typename Element>
inline void QuadTree<Element>::Node::forEach_cullElements(const vector<FrustumPlane>& planes,
	vector<bool>& partsEnabled, void (*function_onEach)(Element* ptr)) {

	bbFrustumState state = boxInsideFrustum(m_position, m_size, planes);
	switch (state) {
	case bbFrustumState::State_Inside:
		// full hit
		// add all elements to the list
		for (size_t i = 0; i < m_elements.size(); i++) {
			size_t index = m_elements[i]->index;
			if (!partsEnabled[index]) {
				partsEnabled[index] = true;
				function_onEach(&m_elements[i]->element);
			}
		}
		break;
	case bbFrustumState::State_Outside:
		// miss
		// add none
		break;
	case bbFrustumState::State_Inbetween:
		// partial hit
		if (expanded) {
			// add children elements
			for (size_t i = 0; i < 4; i++)
				m_children[i]->forEach_cullElements(planes, partsEnabled, function_onEach);
		}
		else {
			// add all elements to the list
			for (size_t i = 0; i < m_elements.size(); i++) {
				size_t index = m_elements[i]->index;
				if (!partsEnabled[index]) {
					partsEnabled[index] = true;
					function_onEach(&m_elements[i]->element);
				}
			}
		}
		break;
	}
}

template <typename Element>
inline QuadTree<Element>::Node::Node(
	float3 position, float3 size, size_t layerMax, size_t layer, size_t reserve) {
	m_position = position;
	m_size = size;
	m_layer = layer;
	m_layerMax = layerMax;
	m_elements.reserve(reserve);
}

template <typename Element> inline QuadTree<Element>::Node::~Node() {}

template <typename Element> inline void QuadTree<Element>::resetFetchState() {
	for (size_t i = 0; i < m_elementParts.size(); i++) {
		m_elementParts[i]->fetched = false;
	}
}

template <typename Element> inline void QuadTree<Element>::log() {
	m_node.log(0);
	ErrorLogger::log("");
}

template <typename Element>
inline void QuadTree<Element>::add(float3 position, float3 size, const Element& element) {
	// add the array
	shared_ptr<ElementPart> part = make_shared<ElementPart>(position, size, element);
	part->index = m_elementParts.size();
	m_elementParts.push_back(part);
	// insert into tree
	bool anyHit = m_node.add(m_elementParts.back().get());
	// remove if missed tree
	if (!anyHit) {
		m_elementParts.pop_back();
	}
}

template <typename Element>
inline void QuadTree<Element>::add(
	float3 lCenterPosition, float3 lHalfSize, float4x4 worldMatrix, const Element& element) {
	// define standard box around center
	float3 points[8] = { float3(-1.f, -1.f, -1.f), float3(1.f, -1.f, -1.f), float3(-1.f, -1.f, 1.f),
		float3(1.f, -1.f, 1.f),

		float3(-1.f, 1.f, -1.f), float3(1.f, 1.f, -1.f), float3(-1.f, 1.f, 1.f),
		float3(1.f, 1.f, 1.f) };
	// tranform points to world space and find bounding box
	float2 MM[3] = { float2(-1, -1), float2(-1, -1),
		float2(-1, -1) }; //.x = min, .y = max, -1 = unset
	float coords[3];
	for (size_t i = 0; i < 8; i++) {
		float3 wp = float3::Transform(lCenterPosition + points[i] * lHalfSize, worldMatrix);
		coords[0] = wp.x, coords[1] = wp.y, coords[2] = wp.z;
		for (size_t j = 0; j < 3; j++) {
			// min
			if (MM[j].x == -1 || coords[j] < MM[j].x)
				MM[j].x = coords[j];
			// max
			if (MM[j].y == -1 || coords[j] > MM[j].y)
				MM[j].y = coords[j];
		}
	}
	// calculate parameters
	float3 position(MM[0].x, MM[1].x, MM[2].x);							  // edge location
	float3 size(MM[0].y - MM[0].x, MM[1].y - MM[1].x, MM[2].y - MM[2].x); // calc differences

	// Add
	add(position, size, element);
}

template <typename Element> inline void QuadTree<Element>::remove(Element& element) {
	for (size_t i = 0; i < m_elementParts.size(); i++) {
		if (m_elementParts[i]->element == element) {
			m_node.remove(m_elementParts[i].get());			  // remove from children
			m_elementParts.erase(m_elementParts.begin() + i); // remove
			// fix indices on elementParts
			for (size_t j = i; j < m_elementParts.size(); j++) {
				m_elementParts[j]->index--;
			}
			break;
		}
	}
}

template <typename Element>
inline vector<Element*> QuadTree<Element>::cullElements(const vector<FrustumPlane>& planes) {

	vector<Element*> elements;
	elements.reserve(m_elementParts.size());
	size_t count = 0;
	m_node.cullElements(planes, elements, count);
	resetFetchState();
	return elements;
}

template <typename Element>
inline vector<Element*> QuadTree<Element>::cullElements(const CubeBoundingBox& bb) {
	vector<Element*> elements;
	elements.reserve(m_elementParts.size());
	size_t count = 0;
	m_node.cullElements(bb, elements, count);
	resetFetchState();
	return elements;
}

template <typename Element>
inline void QuadTree<Element>::foreach_cullElements(
	const vector<FrustumPlane>& planes, void (*onEach)(Element*)) {

	vector<bool> partsEnabled;
	partsEnabled.resize(m_elementParts.size());
	for (size_t i = 0; i < partsEnabled.size(); i++)
		partsEnabled[i] = false;
	m_node.forEach_cullElements(planes, partsEnabled, onEach);
}

template <typename Element>
inline void QuadTree<Element>::initilize(float3 position, float3 size, size_t layerMax) {
	reset();
	m_node = Node(position, size, layerMax, 0, m_elementParts.capacity());
}

template <typename Element> inline void QuadTree<Element>::reset() {
	m_elementParts.clear();
	m_node = Node();
}

template <typename Element> inline void QuadTree<Element>::reserve(size_t size) {
	m_elementParts.reserve(size);
}

template <typename Element>
inline QuadTree<Element>::QuadTree(float3 position, float3 size, size_t layerMax) {
	initilize(position, size, layerMax);
}

template <typename Element> inline QuadTree<Element>::~QuadTree() {}
