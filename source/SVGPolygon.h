#pragma once
#include "SVGElement.h"
#include <vector>
class SVGPolygon : public SVGElement {
private:
   std::vector<PointF> points;
public:
   SVGPolygon() = default;
   SVGPolygon(const std::vector<PointF>& pts, const PaintStyle& style);
   void addPoint(const PointF& point);
   void draw(Graphics* graphics) const override;
};