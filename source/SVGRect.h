#pragma once
#include "SVGElement.h"

class SVGRect : public SVGElement { // Kế thừa từ class SVGElement
public:
	PointF topLeft;
	REAL width;
	REAL height;

	SVGRect(PointF topLeft, REAL width, REAL height, const PaintStyle& s);
	void draw(Graphics* graphics) const override;
};
