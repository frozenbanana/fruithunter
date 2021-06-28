#pragma once
#include <vector>
#include "Translation2DStructures.h"
class UIContainer : public Drawable2D, public vector<shared_ptr<Drawable2D>> {
private:
	void _draw(const Transformation2D& transformation);
	void _imgui_properties();

public:
	float2 getLocalSize() const;

	shared_ptr<Drawable2D> select(float2 mp);
	bool getMatrixChain(Matrix& matrix, shared_ptr<Drawable2D> ptr);

	void imgui_tree(shared_ptr<Drawable2D>& selection);
};
