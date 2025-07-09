#pragma once
#include "SVGElement.h"

class SVGRect : public SVGElement { // Kế thừa từ class SVGElement
public:
   PointF topLeft;
   float width;
   float height;


   SVGRect(PointF topLeft, float width, float height, const PaintStyle &s);
   void draw(Graphics* graphics) const override;
};
