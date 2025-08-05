#include "stdafx.h"
#include "SVGRect.h"

// Function helps double dispatch
void SVGRect::accept(SVGVisitor* visitor){
    visitor->visit(this);
}

//Constructors
SVGRect::SVGRect(PointF topLeft, REAL width, REAL height, const PaintStyle &s)
{
	this->topLeft = topLeft;
	this->width = width;
	this->height = height;
	if (width < 0 || height < 0) {
		this->width = 0; // Đặt chiều rộng về 0 nếu âm
		this->height = 0; // Đặt chiều cao về 0 nếu âm
	}
	this->style = s;
}

// Hàm draw: vẽ hình chữ nhật bằng GDI+
void SVGRect::draw(Graphics* graphics) const {
    if (graphics == nullptr) return;

    // Nếu fillOpacity > 0 thì mới vẽ Fill
    if (style.fillOpacity > 0.0f) {
        BYTE alpha = static_cast<BYTE>(style.fillOpacity * style.fillColor.GetA());
        Color fillColor(alpha,
            style.fillColor.GetRed(),
            style.fillColor.GetGreen(),
            style.fillColor.GetBlue());
        SolidBrush fillBrush(fillColor);
        graphics->FillRectangle(&fillBrush, topLeft.X, topLeft.Y, width, height);
    }

    // Nếu strokeWidth > 0 và strokeOpacity > 0 thì mới vẽ viền
    if (style.strokeWidth > 0.0f) {
        Pen pen(style.strokeColor, static_cast<REAL>(style.strokeWidth));
        graphics->DrawRectangle(&pen, static_cast<REAL>(topLeft.X), static_cast<REAL>(topLeft.Y), width, height);
    }
}

