#pragma once
#include "SVGElement.h"
#include <string>
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

class SVGCircle : public SVGElement {
public:
   PointF center;
   float radius = 0;

   SVGCircle(PointF center, float radius, const PaintStyle &s);
   void draw(Graphics* graphics) const override;
   void accept(SVGVisitor* visitor) override;
   
   // Getters and Setters
   PointF getCenter() const { return center; }
   float getRadius() const { return radius; }
   void setCenter(PointF newCenter) { center = newCenter; }
   void setRadius(float newRadius) { radius = newRadius; }
};