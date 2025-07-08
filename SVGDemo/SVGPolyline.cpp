#include "SVGPolyline.h"

SVGPolyline::SVGPolyline(const std::vector<PointF>& pts, const PaintStyle& s)
    : SVGElement(s), points(pts) {}

void SVGPolyline::addPoint(const Point& point) {
	points.push_back(point);
}