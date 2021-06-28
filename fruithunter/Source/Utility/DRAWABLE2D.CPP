#include "..\Translation2DStructures.h"

void Drawable2D::_imgui_properties() { ImGui::Text("EMPTY"); }

void Drawable2D::draw() {
	if (isDrawing())
		_draw(*this);
}

void Drawable2D::draw(const Transformation2D& matrix) {
	if (isDrawing())
		_draw(Transformation2D::transform(*this, matrix));
}

bool Drawable2D::isDrawing() const { return m_drawing; }

float2 Drawable2D::getSize() const { return getLocalSize() * getScale(); }

BoundingBox2D Drawable2D::getBoundingBox() const {
	BoundingBox2D bb;
	bb.set(getMatrix(), getLocalSize(), getAlignment());
	return bb;
}

BoundingBox2D Drawable2D::getBoundingBox(Matrix parentMatrices) const {
	BoundingBox2D bb;
	bb.set(getMatrix() * parentMatrices, getLocalSize(), getAlignment());
	return bb;
}

void Drawable2D::setDrawState(bool state) { m_drawing = state; }

void Drawable2D::setSize(float2 size) { setScale(size / getLocalSize()); }

void Drawable2D::setAlignment(HorizontalAlignment horizontal, VerticalAlignment vertical) {
	m_horizontalAligment = horizontal;
	m_verticalAlignment = vertical;
}

HorizontalAlignment Drawable2D::getHorizontalAlignment() const { return m_horizontalAligment; }

VerticalAlignment Drawable2D::getVerticalAlignment() const { return m_verticalAlignment; }

float2 Drawable2D::getAlignment() const {
	return float2(m_horizontalAligment, m_verticalAlignment);
}

void Drawable2D::imgui_properties() {
	Transformation2D::imgui_properties();
	int horizontal = m_horizontalAligment + 1;
	int vertical = m_verticalAlignment + 1;
	if (ImGui::Combo("Horizontal Alignment", &horizontal, "Left\0Middle\0Right", 3))
		m_horizontalAligment = HorizontalAlignment(horizontal - 1);
	if (ImGui::Combo("Vertical Alignment", &vertical, "Top\0Center\0Bottom", 3))
		m_verticalAlignment = VerticalAlignment(vertical - 1);
	ImGui::Separator();
	_imgui_properties();
}
