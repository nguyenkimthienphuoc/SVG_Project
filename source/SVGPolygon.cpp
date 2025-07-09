#include "stdafx.h"
#include "SVGPolygon.h"

SVGPolygon::SVGPolygon(const std::vector<PointF>& pts, const PaintStyle& style) {
	points = pts;
	this->style = style;
}
void SVGPolygon::addPoint(const PointF& point) {
	points.push_back(point);
}
void SVGPolygon::draw(Graphics* graphics) const {}