#pragma once
#include "SVGElement.h"
#include <vector>
class SVGPolyline : public SVGElement {
private:
	std::vector<PointF> points;
public:
	SVGPolyline() = default;
	void addPoint(const PointF& point);
	void draw(Graphics* graphics) const override;

};