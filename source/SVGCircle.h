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
};