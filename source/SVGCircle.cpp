#pragma once
#include "stdafx.h"
#include "SVGCircle.h"

//Constructors
SVGCircle::SVGCircle(PointF center, float radius, const PaintStyle& s) : center(center), radius(radius) {
   style = s;
}

void SVGCircle::draw(Graphics* graphics) const {
   //Draw definition of SVGCircle
}
