#include "stdafx.h"
#include "SVGRect.h"

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

void SVGRect::draw(Graphics* graphics) const {
    if (graphics == nullptr) return;

    // Lưu trạng thái gốc của Graphics để khôi phục sau khi transform
    GraphicsState state = graphics->Save();

    // Áp dụng transform nếu có
    graphics->MultiplyTransform(&getTransform());

    // Vẽ phần Fill (nếu có)
    if (style.fillOpacity > 0.0f) {
        BYTE alpha = static_cast<BYTE>(style.fillOpacity * style.fillColor.GetA());
        Color fillColor(
            alpha,
            style.fillColor.GetRed(),
            style.fillColor.GetGreen(),
            style.fillColor.GetBlue()
        );
        SolidBrush fillBrush(fillColor);

        graphics->FillRectangle(
            &fillBrush,
            static_cast<REAL>(topLeft.X),
            static_cast<REAL>(topLeft.Y),
            static_cast<REAL>(width),
            static_cast<REAL>(height)
        );
    }

    // Vẽ phần Stroke (nếu có)
    if (style.strokeWidth > 0.0f && style.strokeOpacity > 0.0f) {
        BYTE alpha = static_cast<BYTE>(style.strokeOpacity * style.strokeColor.GetA());
        Color strokeColor(
            alpha,
            style.strokeColor.GetRed(),
            style.strokeColor.GetGreen(),
            style.strokeColor.GetBlue()
        );
        Pen pen(strokeColor, static_cast<REAL>(style.strokeWidth));

        graphics->DrawRectangle(
            &pen,
            static_cast<REAL>(topLeft.X),
            static_cast<REAL>(topLeft.Y),
            static_cast<REAL>(width),
            static_cast<REAL>(height)
        );
    }

    // Khôi phục lại trạng thái ban đầu để các element khác không bị ảnh hưởng
    graphics->Restore(state);
}
