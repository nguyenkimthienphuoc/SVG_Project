// SVGGroup.h
#pragma once
#include "SVGElement.h"
#include <vector>

class SVGGroup : public SVGElement {
private:
    std::vector<SVGElement*> children;

public:
    SVGGroup(const PaintStyle& style);
    ~SVGGroup();

    void addChild(SVGElement* child);
    void draw(Graphics* graphics) const override;
    const std::vector<SVGElement*>& getChildren() const;

    void accept(SVGVisitor* visitor) override;
};
