#pragma once
#include "Translation2DStructures.h"

enum HorizontalAlignment { AlignLeft = -1, AlignMiddle = 0, AlignRight = 1 };
enum VerticalAlignment { AlignTop = -1, AlignCenter = 0, AlignBottom = 1 };

class Drawable2D : public Transformation2D {
private:

protected:
	virtual void _draw(const Transformation2D& source) = 0;

public:
	void draw();
	void draw(const Transformation2D& matrix);

};
