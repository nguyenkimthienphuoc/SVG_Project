#include "stdafx.h"
SVGLine::SVGLine(float x1, float x2, float y1, float y2, const PaintStyle& s) : x1(x1), x2(x2), y1(y1), y2(y2) {
	this->style = s;
}
void SVGLine::draw(Graphics* graphics) const {
	//Calculate the color with precise opacity
	BYTE alpha = static_cast<BYTE>(style.strokeOpacity * 255);

	//color = stroke color + opacity (combined)
	Gdiplus::Color color(alpha, style.strokeColor.GetRed(), style.strokeColor.GetGreen(), style.strokeColor.GetBlue());
	
	Gdiplus::Pen pen(color, style.strokeWidth);

	graphics->DrawLine(&pen, x1, y1, x2, y2);
}