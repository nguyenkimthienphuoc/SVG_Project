#pragma once
#include "SVGElement.h"

class SVGRect : public SVGElement { // Kế thừa từ class SVGElement
public:
	PointF topLeft;
	REAL width;
	REAL height;

	SVGRect(PointF topLeft, REAL width, REAL height, const PaintStyle& s);
	void draw(Graphics* graphics) const override;
	void accept(SVGVisitor* visitor) override;
	
	// Getters and Setters
	PointF getTopLeft() const { return topLeft; }
	REAL getWidth() const { return width; }
	REAL getHeight() const { return height; }
	void setTopLeft(PointF newTopLeft) { topLeft = newTopLeft; }
	void setWidth(REAL newWidth) { width = newWidth; }
	void setHeight(REAL newHeight) { height = newHeight; }
};
