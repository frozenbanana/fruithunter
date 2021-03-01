#include "Drawable2D.h"

void Drawable2D::draw() { _draw(*this); }

void Drawable2D::draw(const Transformation2D& matrix) {
	_draw(Transformation2D::transform(*this, matrix));
}
