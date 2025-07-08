#include "stdafx.h"
#include "SVGPolygon.h"

void SVGPolygon::addPoint(const PointF& point) {
	points.push_back(point);
}