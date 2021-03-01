#pragma once
#include "Translation2DStructures.h"

enum HorizontalAlignment { Left = -1, Middle = 0, Right = 1 };
enum VerticalAlignment { Top = -1, Center = 0, Bottom = 1 };

class Drawable2D : public Transformation2D {
private:

protected:
	virtual void _draw(const Transformation2D& source) = 0;

public:
	void draw();
	void draw(const Transformation2D& matrix);

};
