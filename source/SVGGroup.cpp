// SVGGroup.cpp
#include "stdafx.h"
#include "SVGGroup.h"
#include "SVGElement.h"

// Function helps double dispatch
void SVGGroup::accept(SVGVisitor* visitor) {
    visitor->visit(this);
}

SVGGroup::SVGGroup(const PaintStyle& s) {
    this->style = s;
}

SVGGroup::~SVGGroup() {
    for (auto child : children)
        delete child;
    children.clear();
}

void SVGGroup::addChild(SVGElement* child) {
    if (child)
        children.push_back(child);
}

void SVGGroup::draw(Graphics* graphics) const {
    // Lưu lại trạng thái gốc của Graphics (bao gồm transform)
    Gdiplus::GraphicsState state = graphics->Save();

    // Áp dụng transform nếu có
    graphics->MultiplyTransform(&getTransform());

    // Vẽ các phần tử con
    for (SVGElement* element : children) {
        if (element) element->draw(graphics);
    }

    // Khôi phục lại trạng thái ban đầu (tránh ảnh hưởng đến phần tử sau)
    graphics->Restore(state);
}

const std::vector<SVGElement*>& SVGGroup::getChildren() const {
    return children;
}
