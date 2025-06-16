#pragma once
#include "SVGElement.h"
#include <vector>
class SVGPolygon : public SVGElement {
private:
	std::vector<Point> points;
public:
	SVGPolygon() = default;
	void addPoint(const Point& point);
	void draw(Graphics* graphics) const override;

};