#pragma once
#include "SVGElement.h"
#include <vector>
class SVGPolygon : public SVGElement {
private:
	std::vector<PointF> points;
public:
	SVGPolygon() = default;
	void addPoint(const PointF& point);
	void draw(Graphics* graphics) const override;

};