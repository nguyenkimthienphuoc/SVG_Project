#include "stdafx.h"
#include "SVGPolyline.h"

SVGPolyline::SVGPolyline(const std::vector<Gdiplus::PointF>& pts, const PaintStyle& s) {
	points = pts;
	this->style = s;
}

void SVGPolyline::addPoint(const Gdiplus::PointF& point) {
   points.push_back(point);
}

void SVGPolyline::draw(Graphics* graphics) const{}