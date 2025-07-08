#pragma once
#include "SVGElement.h"
#include <vector>
class SVGPolyline : public SVGElement {
private:
	std::vector<Point> points;
public:
	SVGPolyline() = default;
	SVGPolyline(const std::vector<PointF>& pts, const PaintStyle& style);
	void addPoint(const Point& point);
	void draw(Graphics* graphics) const override;

};