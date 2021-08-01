#pragma once
#include "GlobalNamespaces.h"
#include <vector>
#include "ErrorLogger.h"

#define OCTREE_SPLITCOUNT 10
#define OCTREE_SPLIT 8 // Should not be changed!

enum BBCollisionState { BB_Inside, BB_Inbetween, BB_Outside };
template <typename Element> class Octree : public vector<shared_ptr<Element>> {
private:
	struct ElementBB {
		size_t m_vecIndex = 0;
		bool m_fetched = false;
		CubeBoundingBox m_boundingBox;
		Element m_element;
		ElementBB(CubeBoundingBox bb, const Element& element, size_t idx) {
			m_vecIndex = idx;
			m_boundingBox = bb;
			m_element = element;
		}
		ElementBB() {}
	};
	class Node {
	private:
		size_t m_layerMax;
		size_t m_layer;
		bool m_expanded = false;
		shared_ptr<Node> m_branches[OCTREE_SPLIT];
		CubeBoundingBox m_boundingBox;

		vector<shared_ptr<ElementBB>> m_elements;

		// --- Private Functions ---
		BBCollisionState boundingBoxInsideFrustum(
			CubeBoundingBox bb, const vector<FrustumPlane>& planes);

	public:
		size_t getElementCount() const;
		size_t getLayerMax() const;
		size_t getLayerIndex() const;
		CubeBoundingBox getBoundingBox() const;

		void split();
		bool add(const shared_ptr<ElementBB>& elementBB);
		bool remove(const shared_ptr<ElementBB>& elementBB);

		void cullElements(
			const vector<FrustumPlane>& planes, vector<Element*>& elements, size_t& count);
		void cullElements(const CubeBoundingBox& bb, vector<Element*>& elements, size_t& count);
		void cullElements(float3 ray_point, float3 ray_direction, vector<Element*>& list);
		void cullElements_limitDistance(float3 ray_point, float3 ray_direction, vector<Element*>& list);

		void log();
		void reset();

		Node(CubeBoundingBox bb = CubeBoundingBox(), size_t layerMax = 1, size_t layer = 0,
			size_t reserve = 0);
	};

	Node m_node;
	vector<shared_ptr<ElementBB>> m_elements;

	void resetFetchState();

public:
	void log();

	bool add(const CubeBoundingBox& bb, const Element& element, bool forceExpand = true);
	bool remove(const Element& element);
	void remove(size_t idx);
	bool updateElement(size_t idx, const CubeBoundingBox& bb, bool forceExpand);

	void changeBoundingBox(const CubeBoundingBox& bb);

	vector<Element*> cullElements(const vector<FrustumPlane>& planes);
	vector<Element*> cullElements(const CubeBoundingBox& bb);
	vector<Element*> getElementsByPosition(float3 pos);
	void cullElements(float3 ray_point, float3 ray_direction, vector<Element*>& list);
	void cullElements_limitDistance(float3 ray_point, float3 ray_direction, vector<Element*>& list);

	void initilize(CubeBoundingBox bb, size_t layerMax, size_t capacity = 0);
	void reset();
	void clear();
	void reserve(size_t size);

	Octree(CubeBoundingBox bb = CubeBoundingBox(), size_t layerMax = 1);
};

template <typename Element> inline void Octree<Element>::resetFetchState() {
	for (size_t i = 0; i < m_elements.size(); i++)
		m_elements[i]->m_fetched = false;
}

template <typename Element> inline void Octree<Element>::log() { m_node.log(); }

template <typename Element>
inline bool Octree<Element>::add(
	const CubeBoundingBox& bb, const Element& element, bool forceExpand) {
	// add the array
	shared_ptr<ElementBB> part = make_shared<ElementBB>(bb, element, m_elements.size());
	// insert into tree
	bool anyHit = m_node.add(part);
	if (anyHit) {
		m_elements.push_back(part);
		return true;
	}
	else {
		if (forceExpand) {
			// expand tree area
			vector<float3> points = m_node.getBoundingBox().getEdgePoints();
			vector<float3> pointsOther = bb.getEdgePoints();
			points.insert(points.end(), pointsOther.begin(), pointsOther.end());
			CubeBoundingBox newBB(points);
			changeBoundingBox(newBB);
			// insert element again
			return add(bb, element, false); // dont need to force area second time
		}
		else
			return false;
	}
}

template <typename Element> inline bool Octree<Element>::remove(const Element& element) {
	for (size_t i = 0; i < m_elements.size(); i++) {
		if (m_elements[i]->m_element == element) {
			remove(i);
			return true;
		}
	}
	return false;
}

template <typename Element> inline void Octree<Element>::remove(size_t idx) {
	m_node.remove(m_elements[idx].get());		// remove from children
	m_elements.erase(m_elements.begin() + idx); // remove
	// fix indices on elementParts
	for (size_t j = idx; j < m_elements.size(); j++) {
		m_elements[j]->m_vecIndex--;
	}
}

template <typename Element>
inline bool Octree<Element>::updateElement(
	size_t idx, const CubeBoundingBox& bb, bool forceExpand) {

	shared_ptr<ElementBB> elementBB = m_elements[idx];
	// update properties
	elementBB->m_boundingBox = bb;
	// remove from tree
	m_node.remove(elementBB);
	// insert update element to tree
	bool anyHit = m_node.add(elementBB);
	// remove if missed tree
	if (anyHit) {
		return true;
	}
	else {
		if (forceExpand) {
			// expand tree area
			vector<float3> points = m_node.getBoundingBox().getEdgePoints();
			vector<float3> pointsOther = bb.getEdgePoints();
			points.insert(points.end(), pointsOther.begin(), pointsOther.end());
			CubeBoundingBox newBB(points);
			changeBoundingBox(newBB);
			// insert element again
			return m_node.add(elementBB);
		}
		else {
			remove(elementBB->m_vecIndex);
			return false;
		}
	}
}

template <typename Element>
inline void Octree<Element>::changeBoundingBox(const CubeBoundingBox& bb) {
	m_node = Node(bb, m_node.getLayerMax(), 0, m_elements.capacity());
	for (size_t i = 0; i < m_elements.size(); i++) {
		bool inserted = m_node.add(m_elements[i]);
		if (!inserted) {
			m_elements.erase(m_elements.begin() + i);
			i--;
		}
	}
}

template <typename Element>
inline vector<Element*> Octree<Element>::cullElements(const vector<FrustumPlane>& planes) {
	vector<Element*> elements;
	elements.reserve(m_elements.size());
	size_t count = 0;
	m_node.cullElements(planes, elements, count);
	resetFetchState();
	return elements;
}

template <typename Element>
inline vector<Element*> Octree<Element>::cullElements(const CubeBoundingBox& bb) {
	vector<Element*> elements;
	elements.reserve(m_elements.size());
	size_t count = 0;
	m_node.cullElements(bb, elements, count);
	resetFetchState();
	return elements;
}

template <typename Element>
inline vector<Element*> Octree<Element>::getElementsByPosition(float3 pos) {
	vector<Element*> elements;
	elements.reserve(m_elements.size());
	CubeBoundingBox bb(pos, float3(0.f));
	size_t count = 0;
	m_node.cullElements(bb, elements, count);
	resetFetchState();
	return elements;
}

template <typename Element>
inline void Octree<Element>::cullElements(
	float3 ray_point, float3 ray_direction, vector<Element*>& list) {
	if (ray_direction.LengthSquared() == 0)
		return;
	list.clear();
	list.reserve(m_elements.size());
	m_node.cullElements(ray_point, ray_direction, list);
	resetFetchState();
}

template <typename Element>
inline void Octree<Element>::cullElements_limitDistance(
	float3 ray_point, float3 ray_direction, vector<Element*>& list) {
	if (ray_direction.LengthSquared() == 0)
		return;
	list.clear();
	list.reserve(m_elements.size());
	m_node.cullElements_limitDistance(ray_point, ray_direction, list);
	resetFetchState();
}

template <typename Element>
inline void Octree<Element>::initilize(CubeBoundingBox bb, size_t layerMax, size_t capacity) {
	clear();
	reserve(capacity);
	m_node = Node(bb, layerMax, 0, capacity);
}

template <typename Element> inline void Octree<Element>::reset() {
	m_elements.clear();
	m_node.reset();
}

template <typename Element> inline void Octree<Element>::clear() {
	m_elements.clear();
	m_node = Node();
}

template <typename Element> inline void Octree<Element>::reserve(size_t size) {
	m_elements.reserve(size);
}

template <typename Element> inline Octree<Element>::Octree(CubeBoundingBox bb, size_t layerMax) {
	initilize(bb, layerMax);
}

template <typename Element>
inline BBCollisionState Octree<Element>::Node::boundingBoxInsideFrustum(
	CubeBoundingBox bb, const vector<FrustumPlane>& planes) {

	// normalized box points
	static float3 boxPoints[8] = { float3(0, 0, 0), float3(1, 0, 0), float3(1, 0, 1),
		float3(0, 0, 1), float3(0, 1, 0), float3(1, 1, 0), float3(1, 1, 1), float3(0, 1, 1) };
	// transform points to world space
	for (size_t i = 0; i < 8; i++) {
		boxPoints[i] = bb.m_position + boxPoints[i] * bb.m_size;
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
			return BB_Outside;
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
		return BB_Inbetween;
	else
		return BB_Inside;
}

template <typename Element> inline size_t Octree<Element>::Node::getElementCount() const {
	return m_elements.size();
}

template <typename Element> inline size_t Octree<Element>::Node::getLayerMax() const {
	return m_layerMax;
}

template <typename Element> inline size_t Octree<Element>::Node::getLayerIndex() const {
	return m_layer;
}

template <typename Element> inline void Octree<Element>::Node::reset() {
	for (size_t i = 0; i < OCTREE_SPLIT; i++)
		m_branches[i].reset();
	m_elements.clear();
	m_expanded = false;
}

template <typename Element> inline CubeBoundingBox Octree<Element>::Node::getBoundingBox() const {
	return m_boundingBox;
}

template <typename Element> inline void Octree<Element>::Node::log() {
	int level = m_layer;
	string str = "";
	for (size_t i = 0; i < level; i++)
		str += "-";
	str += "Node: " + (level == 0 ? "CORE" : to_string(level)) +
		   ", Size: " + to_string(m_elements.size()) +
		   ", Expanded: " + (m_expanded ? "true" : "false");
	// str += "\n";
	ErrorLogger::log(str);
	if (m_expanded) {
		for (size_t i = 0; i < OCTREE_SPLIT; i++)
			m_branches[i]->log(level + 1);
	}
}

template <typename Element> inline void Octree<Element>::Node::split() {
	if (!m_expanded) {
		m_expanded = true;
		// split
		float3 pos = m_boundingBox.m_position;
		float3 halfSize = m_boundingBox.m_size / 2.f;
		size_t cap = ceil((float)m_elements.capacity() / OCTREE_SPLIT);
		static float3 edges[OCTREE_SPLIT] = {
			float3(0, 0, 0),
			float3(0, 1, 0),
			float3(0, 0, 1),
			float3(0, 1, 1),
			float3(1, 0, 0),
			float3(1, 1, 0),
			float3(1, 0, 1),
			float3(1, 1, 1),
		};
		for (size_t i = 0; i < OCTREE_SPLIT; i++) {
			CubeBoundingBox bb(pos + halfSize * edges[i], halfSize);
			m_branches[i] = make_shared<Node>(bb, m_layerMax, m_layer + 1, cap);
		}

		// add existing elements to children
		for (size_t i = 0; i < OCTREE_SPLIT; i++) {
			for (size_t j = 0; j < m_elements.size(); j++)
				m_branches[i]->add(m_elements[j]);
		}
	}
}

template <typename Element>
inline bool Octree<Element>::Node::add(const shared_ptr<ElementBB>& elementBB) {
	if (m_boundingBox.intersect(elementBB->m_boundingBox)) {
		// split if to big
		if (!m_expanded && m_layer < m_layerMax && m_elements.size() > OCTREE_SPLITCOUNT - 1)
			split();
		// add
		m_elements.push_back(elementBB);
		// add for all children
		if (m_expanded) {
			for (size_t i = 0; i < OCTREE_SPLIT; i++)
				m_branches[i]->add(elementBB);
		}
		return true;
	}
	return false;
}

template <typename Element>
inline bool Octree<Element>::Node::remove(const shared_ptr<ElementBB>& elementBB) {
	for (size_t i = 0; i < m_elements.size(); i++) {
		if (m_elements[i].get() == elementBB.get()) {
			// remove
			m_elements.erase(m_elements.begin() + i);
			// remove from children
			if (m_expanded) {
				for (size_t j = 0; j < OCTREE_SPLIT; j++)
					m_branches[j]->remove(elementBB);
			}
			break;
		}
	}
}

template <typename Element>
inline Octree<Element>::Node::Node(
	CubeBoundingBox bb, size_t layerMax, size_t layer, size_t reserve) {

	m_boundingBox = bb;
	m_layerMax = layerMax;
	m_layer = layer;
	m_elements.reserve(reserve);
}

template <typename Element>
inline void Octree<Element>::Node::cullElements(
	const vector<FrustumPlane>& planes, vector<Element*>& elements, size_t& count) {
	BBCollisionState state = boundingBoxInsideFrustum(m_boundingBox, planes);
	count++;
	switch (state) {
	case BBCollisionState::BB_Inside:
		// full hit, add all elements to the list
		for (size_t i = 0; i < m_elements.size(); i++) {
			if (!m_elements[i]->m_fetched) {
				m_elements[i]->m_fetched = true;
				elements.push_back(&m_elements[i]->m_element);
			}
		}
		break;
	case BBCollisionState::BB_Outside:
		// miss, add none
		break;
	case BBCollisionState::BB_Inbetween:
		// partial hit
		if (m_expanded) {
			// add children elements
			for (size_t i = 0; i < OCTREE_SPLIT; i++)
				m_branches[i]->cullElements(planes, elements, count);
		}
		else {
			// add all elements to the list
			for (size_t i = 0; i < m_elements.size(); i++) {
				if (!m_elements[i]->m_fetched) {
					m_elements[i]->m_fetched = true;
					elements.push_back(&m_elements[i]->m_element);
				}
			}
		}
		break;
	}
}

template <typename Element>
inline void Octree<Element>::Node::cullElements(
	const CubeBoundingBox& bb, vector<Element*>& elements, size_t& count) {
	BBCollisionState state =
		m_boundingBox.intersect(bb) ? BBCollisionState::BB_Inbetween : BBCollisionState::BB_Outside;
	count++;
	switch (state) {
	case BBCollisionState::BB_Inside:
		// full hit, add all elements to the list
		for (size_t i = 0; i < m_elements.size(); i++) {
			if (!m_elements[i]->m_fetched) {
				m_elements[i]->m_fetched = true;
				elements.push_back(&m_elements[i]->m_element);
			}
		}
		break;
	case BBCollisionState::BB_Outside:
		// miss, add none
		break;
	case BBCollisionState::BB_Inbetween:
		// partial hit
		if (m_expanded) {
			// add children elements
			for (size_t i = 0; i < OCTREE_SPLIT; i++)
				m_branches[i]->cullElements(bb, elements, count);
		}
		else {
			// add all elements to the list
			for (size_t i = 0; i < m_elements.size(); i++) {
				if (!m_elements[i]->m_fetched) {
					m_elements[i]->m_fetched = true;
					elements.push_back(&m_elements[i]->m_element);
				}
			}
		}
		break;
	}
}

template <typename Element>
inline void Octree<Element>::Node::cullElements(
	float3 ray_point, float3 ray_direction, vector<Element*>& list) {
	float t = 0;
	if (m_boundingBox.intersect(ray_point, ray_direction, t)) {
		if (m_expanded) {
			// add children elements
			for (size_t i = 0; i < OCTREE_SPLIT; i++)
				m_branches[i]->cullElements(ray_point, ray_direction, list);
		}
		else {
			// add all elements to the list
			for (size_t i = 0; i < m_elements.size(); i++) {
				if (!m_elements[i]->m_fetched) {
					m_elements[i]->m_fetched = true;
					list.push_back(&m_elements[i]->m_element);
				}
			}
		}
	}
}

template <typename Element>
inline void Octree<Element>::Node::cullElements_limitDistance(
	float3 ray_point, float3 ray_direction, vector<Element*>& list) {
	float t = 0;
	if (m_boundingBox.intersect(ray_point, ray_direction, t)) {
		if (t < ray_direction.Length()) {
			if (m_expanded) {
				// add children elements
				for (size_t i = 0; i < OCTREE_SPLIT; i++)
					m_branches[i]->cullElements(ray_point, ray_direction, list);
			}
			else {
				// add all elements to the list
				for (size_t i = 0; i < m_elements.size(); i++) {
					if (!m_elements[i]->m_fetched) {
						m_elements[i]->m_fetched = true;
						list.push_back(&m_elements[i]->m_element);
					}
				}
			}
		}
	}
}
