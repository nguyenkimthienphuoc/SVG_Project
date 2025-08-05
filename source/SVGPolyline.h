#pragma once
#include "SVGElement.h"
#include <vector>
class SVGPolyline : public SVGElement {
private:
   std::vector<PointF> points;
public:
   SVGPolyline() = default;
   SVGPolyline(const std::vector<PointF>& pts, const PaintStyle& style);
   void addPoint(const PointF& point);
   void draw(Graphics* graphics) const override;
   void accept(SVGVisitor* vistor) override;

   // Getter
   const std::vector<PointF>& getPoints() const { return points; }
};