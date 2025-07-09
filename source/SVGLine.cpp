#include "stdafx.h"
SVGLine::SVGLine(float x1, float x2, float y1, float y2, const PaintStyle& s) : x1(x1), x2(x2), y1(y1), y2(y2) {
	this->style = s;
}
void SVGLine::draw(Graphics* graphics) const {}