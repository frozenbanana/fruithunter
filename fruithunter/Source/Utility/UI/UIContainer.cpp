#include "UIContainer.h"

void UIContainer::_draw(const Transformation2D& transformation) {
	for (size_t i = 0; i < size(); i++) {
		if (at(i).get() != nullptr)
			at(i)->draw(transformation);
	}
}

void UIContainer::_imgui_properties() {}

float2 UIContainer::getLocalSize() const { return float2(50, 50); }

shared_ptr<Drawable2D> UIContainer::select(float2 mp) {
	float2 mp_transformed = float2::Transform(mp, getMatrix().Invert());
	for (size_t i = 0; i < size(); i++) {
		UIContainer* c = dynamic_cast<UIContainer*>(at(i).get());
		if (c == nullptr) {
			if (at(i)->getBoundingBox().isInside(mp_transformed)) {
				return at(i);
			}
		}
		else {
			shared_ptr<Drawable2D> e = c->select(mp_transformed);
			if (e.get() != nullptr)
				return e;
		}
	}
	return shared_ptr<Drawable2D>();
}

bool UIContainer::getMatrixChain(Matrix& matrix, shared_ptr<Drawable2D> ptr) {
	for (size_t i = 0; i < size(); i++) {
		if (at(i).get() == ptr.get()) {
			matrix = matrix * getMatrix();
			return true;
		}
		UIContainer* c = dynamic_cast<UIContainer*>(at(i).get());
		if (c != nullptr) {
			if (c->getMatrixChain(matrix, ptr)) {
				matrix = matrix * getMatrix();
				return true;
			}
		}
	}
	return false;
}

void UIContainer::imgui_tree(shared_ptr<Drawable2D>& selection) {
	bool selectedThisFrame = false;
	for (size_t i = 0; i < size(); i++) {
		UIContainer* c = dynamic_cast<UIContainer*>(at(i).get());
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
								   ImGuiTreeNodeFlags_OpenOnDoubleClick |
								   ImGuiTreeNodeFlags_SpanAvailWidth;
		if (selection.get() == at(i).get())
			flags |= ImGuiTreeNodeFlags_Selected;
		if (c == nullptr) {
			bool open = ImGui::TreeNodeEx(
				at(i).get(), flags | ImGuiTreeNodeFlags_Leaf, ("Item" + to_string(i)).c_str());
			if (ImGui::IsItemClicked())
				selection = at(i);
			if (open) {
				ImGui::TreePop();
			}
		}
		else {
			bool open = ImGui::TreeNodeEx(at(i).get(), flags, ("Container" + to_string(i)).c_str());
			if (ImGui::IsItemClicked())
				selection = at(i);
			if (open) {
				c->imgui_tree(selection);
				ImGui::TreePop();
			}
		}
	}
}
