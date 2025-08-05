#include "stdafx.h"
// Function helps double dispatch
void SVGLine::accept(SVGVisitor* visitor){
	visitor->visit(this);
}

SVGLine::SVGLine(float x1, float x2, float y1, float y2, const PaintStyle& s) : x1(x1), x2(x2), y1(y1), y2(y2) {
	this->style = s;
}
void SVGLine::draw(Graphics* graphics) const {
	// Lưu trạng thái gốc của Graphics để khôi phục sau khi transform
	GraphicsState state = graphics->Save();

	// Áp dụng transform nếu có
	graphics->MultiplyTransform(&getTransform());

	//Calculate the color with precise opacity
	BYTE alpha = static_cast<BYTE>(style.strokeOpacity * style.strokeColor.GetA());

	//color = stroke color + opacity (combined)
	Gdiplus::Color color(alpha, style.strokeColor.GetRed(), style.strokeColor.GetGreen(), style.strokeColor.GetBlue());
	
	Gdiplus::Pen pen(color, style.strokeWidth);

	graphics->DrawLine(&pen, x1, y1, x2, y2);

	// Khôi phục lại trạng thái ban đầu để các element khác không bị ảnh hưởng
	graphics->Restore(state);
}